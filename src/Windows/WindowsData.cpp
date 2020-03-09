#include "WindowsData.h"

#ifdef _WIN32

#include "D3D11Info.h"
#include "../../include/PDMData.h"
#include "../Gatherer.h"
#include "../Wine/Wine.h"
#include "VulkanInfo.h"

#include <algorithm>
#include <sstream>
#include <atlstr.h>
#include <Lmcons.h>
#include <intrin.h>
#include <iomanip>
#include <Iphlpapi.h>

#pragma comment(lib, "IPHLPAPI.lib")


namespace PDM
{
	struct NetworkAdapterInfo
	{
		std::string name;
		std::string description;
		std::string macAddress;
	};

	std::string GetStringFromReg(CString keyValName)
	{
		const HKEY parent = HKEY_LOCAL_MACHINE;
		const CString keyName = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";

		CRegKey key;
		std::string out;

		if (key.Open(parent, keyName, KEY_READ) == ERROR_SUCCESS)
		{
			ULONG len = 256;
			CString str;
			DWORD value;

			bool success = key.QueryStringValue(keyValName, str.GetBuffer(len), &len) == ERROR_SUCCESS;
			str.ReleaseBuffer();

			if (success)
				out = str.GetString();
			else if (key.QueryDWORDValue(keyValName, value) == ERROR_SUCCESS)
				out = std::to_string(value);

			key.Close();
		}

		return out;
	}

	Bitness GetOSBitnessInternal()
	{
#ifdef _WIN64
		return Bitness::BITNESS_64;
#else
		BOOL isWow = false;
		IsWow64Process(GetCurrentProcess(), &isWow);
		return isWow ? Bitness::BITNESS_64 : Bitness::BITNESS_32;
#endif
	}

	OS GetOSType()
	{
		return IsWine() ? OS::WINE : OS::WINDOWS;
	}

	std::string GetOSName()
	{
		return IsWine() ? "" : GetStringFromReg(L"ProductName");
	}

	std::string GetOSMajorVersion()
	{
		return IsWine() ? "" : GetStringFromReg(L"CurrentMajorVersionNumber");
	}

	std::string GetOSMinorVersion()
	{
		return IsWine() ? "" : GetStringFromReg(L"CurrentMinorVersionNumber");
	}

	std::string GetOSBuildNumber()
	{
		return IsWine() ? "" : GetStringFromReg(L"CurrentBuild");
	}

	std::string GetOSKernelVersion()
	{
		return IsWine() ? "" : GetStringFromReg(L"CurrentVersion");
	}

	std::string GetMachineName()
	{
		constexpr auto INFO_BUFFER_SIZE = 1024;
		TCHAR  infoBuf[INFO_BUFFER_SIZE];
		DWORD  bufCharCount = INFO_BUFFER_SIZE;
		return GetComputerName(infoBuf, &bufCharCount) ? infoBuf : "";
	}

	std::string GetUsername()
	{
		char username[UNLEN + 1];
		DWORD username_len = UNLEN + 1;
		return GetUserName(username, &username_len) ? username : "";
	}

	unsigned GetScreenCount()
	{
		return GetSystemMetrics(SM_CMONITORS);
	}

	uint64_t GetTotalMemory()
	{
		MEMORYSTATUSEX status;
		status.dwLength = DWORD(sizeof(status));
		return GlobalMemoryStatusEx(&status) ? status.ullTotalPhys : 0;
	}

	std::string GetMachineUuid()
	{
		REGSAM access = KEY_READ;
#ifndef _WIN64
		if (GetOSBitnessInternal() != Bitness.BITNESS_32)
			access |= KEY_WOW64_64KEY;
#endif

		HKEY key;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", 0, access, &key) != ERROR_SUCCESS) return "";

		DWORD type;
		char guid[256];
		DWORD size = DWORD(sizeof(guid));
		LSTATUS status = RegQueryValueEx(key, "MachineGuid", nullptr, &type, reinterpret_cast<LPBYTE>(guid), &size);
		RegCloseKey(key);

		return status == ERROR_SUCCESS && type == REG_SZ ? guid : "";
	}

	std::vector<NetworkAdapterInfo> GetAdapterInfo()
	{
		ULONG l = 0;
		DWORD res = GetAdaptersInfo(0, &l);
		if (res != ERROR_BUFFER_OVERFLOW) return {};
		std::vector<char> buf(l);
		res = GetAdaptersInfo((IP_ADAPTER_INFO*)&buf[0], &l);
		if (res != ERROR_SUCCESS) return {};

		IP_ADAPTER_INFO* pi = (IP_ADAPTER_INFO*)&buf[0];
		std::vector<NetworkAdapterInfo> adapters;
		for (; pi; pi = pi->Next)
		{
			std::stringstream stream;
			for (unsigned i = 0; i < 6; i++)
			{
				if (i) stream << "-";
				stream << std::hex << static_cast<unsigned>(pi->Address[i]);
			}
			std::string macAddr(stream.str());
			std::transform(macAddr.begin(), macAddr.end(), macAddr.begin(), [](char c) { return static_cast<char>(std::toupper(c)); });

			adapters.push_back
			({
				pi->AdapterName,
				pi->Description,
				macAddr,
			});
		}

		return adapters;
	}

	bool IsHypervisorGuestVM()
	{
		return CPUID(1).ECX() & 0x80000000;
	}

	bool IsVMExecutionTiming()
	{
		// Both these values are arbitrary (needs investigation)
		const unsigned THRESHOLD = 500;
		const unsigned RUNS = 1024;

		uint64_t average{ 0 };
		for (unsigned i = 0; i < RUNS; i++)
		{
#if _WIN64
			auto time1 = __rdtsc();
			auto time2 = __rdtsc();
#else
			unsigned time1 = 0;
			unsigned time2 = 0;
			__asm
			{
				RDTSC
				MOV time1, EAX
				RDTSC
				MOV time2, EAX
			}
#endif
			average += time2 - time1;
		}

		average /= RUNS;
		return average > THRESHOLD;
	}

	bool IsRunningVM()
	{
		return IsHypervisorGuestVM() || IsVMExecutionTiming();
	}

	bool IsRemoteSession()
	{
		return GetSystemMetrics(SM_REMOTESESSION);
	}

#pragma warning(disable:26812)
	constexpr const char* D3DFeatureSupportToString(D3D_FEATURE_LEVEL support)
#pragma warning(default:26812)
	{
		switch (support)
		{
		case D3D_FEATURE_LEVEL_12_1:
			return "12.1";
		case D3D_FEATURE_LEVEL_12_0:
			return "12.0";
		case D3D_FEATURE_LEVEL_11_1:
			return "11.1";
		case D3D_FEATURE_LEVEL_11_0:
			return "11.0";
		case D3D_FEATURE_LEVEL_10_1:
			return "10.1";
		case D3D_FEATURE_LEVEL_10_0:
			return "10.0";
		case D3D_FEATURE_LEVEL_9_3:
			return "9.3";
		case D3D_FEATURE_LEVEL_9_2:
			return "9.2";
		case D3D_FEATURE_LEVEL_9_1:
			return "9.1";
		default:
			return "UNKNOWN";
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

	SubItem GetWindowsSubItems()
	{
		if (IsWine()) return {};

		D3D11Info d3dInfo = GetD3DInfo();

		std::vector<SubItem> monitors;
		for (auto& m : d3dInfo.monitors)
		{
			monitors.push_back
			({
				"MONITOR",
				{},
				{
					{"VERTICAL_RES",   std::to_string(m.width)},
					{"HORIZONTAL_RES", std::to_string(m.height)},
					{"BITS_PER_COLOR", std::to_string(m.bitsPerColor)},
				}
			});
		}

		std::vector<SubItem> gpus;
		for (auto& gpu : d3dInfo.adapters)
		{
			gpus.push_back
			({
				"GPU",
				{},
				{
					{"DESCRIPTION",    gpu.description},
					{"VENDOR_ID",      std::to_string(gpu.vendorID)},
					{"DEVICE_ID",      std::to_string(gpu.deviceID)},
					{"SUBSYSTEM_ID",   std::to_string(gpu.subSystemID)},
					{"DRIVER_DATE",    gpu.driverDate},
					{"DRIVER_VENDOR",  gpu.driverVendor},
					{"DRIVER_VERSION", gpu.driverVersionString},
					{"REVISION",       std::to_string(gpu.revision)},
				}
			});
		}

		std::vector<SubItem> networkAdapters;
		for (auto& adapter : GetAdapterInfo())
		{
			networkAdapters.push_back
			({
				"ADAPTER",
				{},
				{
					{"NAME",        adapter.name},
					{"DESCRIPTION", adapter.description},
					{"MAC_ADDRESS", adapter.macAddress},
				}
			});
		}

		VulkanProperties vulkanProperties = GetVulkanProperties();

		return
		{
			"WINDOWS",
			{
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
				{
					{"IS_REMOTE_SESSION",      IsRemoteSession() ? "YES" : "NO"},
					{"D3D_HIGHEST_SUPPORT",    D3DFeatureSupportToString(d3dInfo.maxSupportedFeatureLevel)},
					{"VULKAN_SUPPORTED",       VulkanSupportToString(vulkanProperties.support)},
					{"VULKAN_HIGHEST_SUPPORT", vulkanProperties.version},
				},
			}
		};
	}
}

#else

namespace PDM
{
	SubItem GetWindowsSubItems()
	{
		return {};
	}
}

#endif
