#include "gatherer.h"
#include "defines.h"
#include "../include/pdm.h"

#include <string>
#include <thread>
#include <time.h>

namespace PDM
{
	struct CPUInfo
	{
		int model{ 0 };
		int stepping{ 0 };
		std::string vendor;
		std::string brand;
		Bitness bitness;
	};

	Bitness GetOSBitnessInternal();


	constexpr const char* GetVersion()
	{
		return "1.0.0";
	}

	CPUInfo GetCPUInfo()
	{
		// Only works on x86/x64/IA32/IA64 architectures

		Bitness bitness;

		CPUID id8(0x80000000);
		if (id8.EAX() >= 0x80000001u)
		{
			CPUID id81(0x80000001);
			bitness = id81.EDX() & (1 << 29) ? Bitness::BITNESS_64 : Bitness::BITNESS_32;
		}
		else
		{
			bitness = Bitness::BITNESS_32;
		}
		
		std::string brand;

		if (id8.EAX() >= 0x80000004u)
		{
			for (unsigned i = 0; i < 3; i++)
			{
				CPUID id(0x80000002u + i);

				brand += std::string(reinterpret_cast<const char*>(&id.EAX()), 4);
				brand += std::string(reinterpret_cast<const char*>(&id.EBX()), 4);
				brand += std::string(reinterpret_cast<const char*>(&id.ECX()), 4);
				brand += std::string(reinterpret_cast<const char*>(&id.EDX()), 4);
			}

			trim(brand);
		}

		CPUID id0(0);
		std::string vendor;
		vendor += std::string(reinterpret_cast<const char*>(&id0.EBX()), 4);
		vendor += std::string(reinterpret_cast<const char*>(&id0.EDX()), 4);
		vendor += std::string(reinterpret_cast<const char*>(&id0.ECX()), 4);

		int model = 0;
		int stepping = 0;

		if (id0.EAX() > 0)
		{
			CPUID id1(1);
			model = (id1.EAX() >> 4) & 0xf;
			stepping = id1.EAX() & 0xf;
		}

		return { model, stepping, vendor, brand, bitness };
	}

	bool IsVMExecutionTiming()
	{
		// Both these values are arbitrary (needs investigation)
		// Average time on a modern processor natively is around 25 cycles
		const unsigned THRESHOLD_TIME = 500;
		const unsigned RUNS = 1024;

		unsigned thresholdCrossings = 0;
		uint64_t time1 = 0;
		uint64_t time2 = 0;
		
		for (unsigned i = 0; i < RUNS; i++)
		{
#if _WIN64
			time1 = __rdtsc();
			time2 = __rdtsc();
#else
			asm volatile("RDTSC" : "=a" (time1));
			asm volatile("RDTSC" : "=a" (time2));
#endif
			if (time2 - time1 > THRESHOLD_TIME) thresholdCrossings++;
		}

		return thresholdCrossings > (RUNS / 2);
	}

	bool IsHypervisorGuestVM()
	{
		return CPUID(1).ECX() & 0x80000000;
	}

	bool IsRunningVM()
	{
		return IsHypervisorGuestVM() || IsVMExecutionTiming();
	}

	constexpr Bitness GetProcessBitness()
	{
		switch (sizeof(void*))
		{
		case 8:
			return Bitness::BITNESS_64;
		case 4:
			return Bitness::BITNESS_32;
		default:
			return Bitness::BITNESS_UNKNOWN;
		}
	}

	Bitness GetOSBitness()
	{
		if (GetProcessBitness() == Bitness::BITNESS_64) return Bitness::BITNESS_64;

		return GetOSBitnessInternal();
	}

	Bitness GetCPUBitness(CPUInfo info)
	{
		if (GetOSBitness() == Bitness::BITNESS_64) return Bitness::BITNESS_64;

		return info.bitness;
	}

	TimeStamp GetCurrentTime()
	{

		time_t rawtime;
		time(&rawtime);
		TimeStamp time{ {0} };

#if _WIN32
		localtime_s(&time, &rawtime);
#else
		localtime_r(&rawtime, &time);
#endif

		return time;
	}

	constexpr const char* BitnessToString(Bitness bitness)
	{
		switch (bitness)
		{
		case Bitness::BITNESS_64:
			return "x64";
		case Bitness::BITNESS_32:
			return "x32";
		case Bitness::BITNESS_UNKNOWN:
		default:
			return "Unknown";
		}
	}

	constexpr const char* OSToString(OS os)
	{
		switch (os)
		{
		case OS::WINDOWS:
			return "Windows";
		case OS::MACOS:
			return "macOS";
		case OS::WINE:
			return "Wine";
		case OS::UNKNOWN:
		default:
			return "Unknown";
		}
	}

	constexpr const char* VulkanSupportToString(VulkanSupport support)
	{
		switch (support)
		{
		case VulkanSupport::SUPPORTED:
			return "YES";
		case VulkanSupport::UNSUPPORTED:
			return "NO";
		case VulkanSupport::UNKNOWN:
		default:
			return "UNKNOWN";
		}
	}

	std::string TimestampToString(const TimeStamp& timestamp)
	{
		const int MAX_SIZE = 20;
		char time[MAX_SIZE];
		strftime(time, MAX_SIZE, "%F %T", &timestamp);
		return time;
	}

	PDMData GatherData()
	{
		TimeStamp timestamp = GetCurrentTime();
		CPUInfo cpuinfo = GetCPUInfo();

		std::vector<SubItem> monitors;
		for (auto& monitor : GetMonitorsInfo())
		{
			monitors.push_back
			({
				"MONITOR",
				{},
				{
					{"VERTICAL_RES",   std::to_string(monitor.width)},
					{"HORIZONTAL_RES", std::to_string(monitor.height)},
					{"BITS_PER_COLOR", std::to_string(monitor.bitsPerColor)},
				}
			});
		}

		std::vector<SubItem> gpus;
		for (auto& gpu : GetGPUInfo())
		{
			gpus.push_back
			({
				"GPU",
				{},
				{
					{"DESCRIPTION",    gpu.description},
					{"VENDOR_ID",      std::to_string(gpu.vendorID)},
					{"DEVICE_ID",      std::to_string(gpu.deviceID)},
					{"REVISION",       std::to_string(gpu.revision)},
					{"DRIVER_DATE",    gpu.driverDate},
					{"DRIVER_VENDOR",  gpu.driverVendor},
					{"DRIVER_VERSION", gpu.driverVersionString},
				}
			});
		}

		std::vector<SubItem> networkAdapters;
		for (auto& adapter : GetNetworkAdapterInfo())
		{
			networkAdapters.push_back
			({
				"ADAPTER",
				{},
				{
					{"NAME",        adapter.name},
					{"MAC_ADDRESS", adapter.macAddress},
					{"UUID",        adapter.uuid},
				}
			});
		}
		
		VulkanProperties vulkanProperties = GetVulkanProperties();

		return
		{
			{
				"DATA",
				{
					{
						"GENERAL",
						{
							{
								"PROCESS",
								{},
								{
									{"VERSION",   GetVersion()},
									{"TIMESTAMP", TimestampToString(timestamp)},
									{"BITNESS",   BitnessToString(GetProcessBitness())},
								}
							},
							{
								"OS",
								{
									{
										"GRAPHICS_APIS",
										{},
										{
											{"METAL_SUPPORTED",        GetMetalSupported() ? "YES" : "NO"},
											{"VULKAN_SUPPORTED",       VulkanSupportToString(vulkanProperties.support)},
											{"VULKAN_HIGHEST_SUPPORT", vulkanProperties.version},
											{"D3D_HIGHEST_SUPPORT",    GetD3DHighestSupport()},
										},
									},
									{
										"WINE",
										{},
										{
											{"VERSION", GetWineVersion()},
											{"HOST_OS", GetWineHostOs()},
										},
									},
								},
								{
									{"TYPE",              OSToString(GetOSType())},
									{"NAME",              GetOSName()},
									{"BITNESS",           BitnessToString(GetOSBitness())},
									{"MAJOR_VERSION",     std::to_string(GetOSMajorVersion())},
									{"MINOR_VERSION",     std::to_string(GetOSMinorVersion())},
									{"BUILD_NUMBER",      std::to_string(GetOSBuildNumber())},
									{"KERNEL_VERSION",    GetOSKernelVersion()},
									{"USERNAME",          GetUsername()},
									{"USER_LOCALE",       GetUserLocale()},
									{"IS_REMOTE_SESSION", IsRemoteSession() ? "YES" : "NO"},
								}
							},
							{
								"MACHINE",
								{
									{
										"CPU",
										{},
										{
											{"BITNESS",            BitnessToString(GetCPUBitness(cpuinfo))},
											{"LOCIGAL_CORE_COUNT", std::to_string(std::thread::hardware_concurrency())},
											{"BRAND",              cpuinfo.brand},
											{"VENDOR",             cpuinfo.vendor},
											{"MODEL",              std::to_string(cpuinfo.model)},
											{"STEPPING",           std::to_string(cpuinfo.stepping)},
										}
									},
									{
										"MONITORS",
										monitors,
										{},
									},
									{
										"GPUS",
										gpus,
										{},
									},
									{
										"NETWORK_ADAPTERS",
										networkAdapters,
										{},
									},
								},
								{
									{"MODEL",          GetHardwareModel()},
									{"NAME",           GetMachineName()},
									{"UUID",           GetMachineUuid()},
									{"TOTAL_MEMORY",   std::to_string(GetTotalMemory())},
									{"MONITOR_COUNT",  std::to_string(GetMonitorCount())},
									{"IS_VM",          IsRunningVM() ? "YES" : "NO"},
								}
							},
						},
						{}
					},
				},
				{}
			},
			timestamp
		};
	}
}
