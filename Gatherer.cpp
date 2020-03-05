#include "Gatherer.h"
#include "Wine/Wine.h"

#include <intrin.h>
#include <string>
#include <thread>

#ifdef _WIN32
#include "Windows/WindowsData.h"
#endif

namespace PDM
{
	struct CPUInfo
	{
		int model;
		int stepping;
		std::string vendor;
		std::string brand;
		Bitness bitness;
	};

	Bitness GetOSBitnessInternal();


	std::string GetVersion()
	{
		return "1.0.0";
	}

	CPUInfo GetCPUInfo()
	{
		// Only works on x86/x64/IA32/IA64 architectures

		Bitness bitness;

		int cpuInfo[4] = { 0 };
		__cpuid(cpuInfo, 0x80000000);
		int idMax = cpuInfo[0];
		if (idMax >= 0x80000001)
		{
			__cpuidex(cpuInfo, 0x80000001, 0);
			bitness = cpuInfo[3] & (1 << 29) ? Bitness::BITNESS_64 : Bitness::BITNESS_32;
		}
		else
		{
			bitness = Bitness::BITNESS_32;
		}

		char brand[0x40] = { 0 };

		if (idMax >= 0x80000004)
		{
			__cpuidex(reinterpret_cast<int*>(brand), 0x80000002, 0);
			__cpuidex(reinterpret_cast<int*>(brand + 16), 0x80000003, 0);
			__cpuidex(reinterpret_cast<int*>(brand + 32), 0x80000004, 0);
		}

		char vendor[0x20] = { 0 };
		__cpuidex(cpuInfo, 0, 0);
		*reinterpret_cast<int*>(vendor) = cpuInfo[1];
		*reinterpret_cast<int*>(vendor + 4) = cpuInfo[3];
		*reinterpret_cast<int*>(vendor + 8) = cpuInfo[2];

		int model = 0;
		int stepping = 0;

		__cpuid(cpuInfo, 0);
		idMax = cpuInfo[0];
		if (idMax > 0)
		{
			__cpuidex(cpuInfo, 1, 0);
			model = (cpuInfo[0] >> 4) & 0xf;
			stepping = cpuInfo[0] & 0xf;
		}

		return { model, stepping, vendor, brand, bitness };
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

	const std::string BitnessToString(Bitness bitness)
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

	const std::string OSToString(OS os)
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

	std::string TimestampToString(const TimeStamp& timestamp)
	{
		const int MAX_SIZE = 20;
		char time[MAX_SIZE];
		strftime(time, MAX_SIZE, "%F %T", &timestamp);
		return time;
	}

	const PDMData GatherData()
	{
		time_t rawtime;
		time(&rawtime);
		struct tm timeinfo{0};
		localtime_s(&timeinfo, &rawtime);
		TimeStamp timestamp{ timeinfo };

		CPUInfo cpuinfo = GetCPUInfo();

		return PDMData
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
								"OS",
								{},
								{
									{"TYPE",           OSToString(GetOSType())},
									{"NAME",           GetOSName()},
									{"BITNESS",        BitnessToString(GetOSBitness())},
									{"MAJOR_VERSION",  GetOSMajorVersion()},
									{"MINOR_VERSION",  GetOSMinorVersion()},
									{"BUILD_NUMBER",   GetOSBuildNumber()},
									{"KERNEL_VERSION", GetOSKernelVersion()},
									{"USERNAME",       GetUsername()},
								}
							},
							{
								"MACHINE",
								{},
								{
									{"MACHINE_NAME", GetMachineName()},
									{"MACHINE_UUID", GetMachineUuid()},
									{"TOTAL_MEMORY", std::to_string(GetTotalMemory())},
									{"SCREEN_COUNT", std::to_string(GetScreenCount())},
									{"IS_VM",        IsRunningVM() ? "YES" : "NO"},
								}
							},
						}
					},
					GetWindowsSubItems(),
					{},
					GetWineSubItems(),
				}
			},
			timestamp
		};
	}
}