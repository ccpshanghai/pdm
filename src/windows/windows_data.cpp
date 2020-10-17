#include <string>

#if _WIN32

#include "d3d11_info.h"
#include "../../include/pdm_data.h"
#include "../defines.h"
#include "../gatherer.h"
#include "../utilities.h"

#include <algorithm>
#include <sstream>
#include <clocale>
#include <atlstr.h>
#include <Lmcons.h>
#include <intrin.h>
#include <iomanip>
#include <Iphlpapi.h>

#pragma comment(lib, "IPHLPAPI.lib")


namespace PDM
{
	const std::wstring CURRENT_VERSION_KEY = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";

	std::string GetStringFromReg(const std::wstring& keyName, const std::wstring& keyValName)
	{
		const HKEY parent = HKEY_LOCAL_MACHINE;

		wchar_t data[256]{1};
		auto size = sizeof(data);
		auto len = (LPDWORD)&size;
		LONG retCode = RegGetValueW(
			parent,
			keyName.c_str(),
			keyValName.c_str(),
			RRF_RT_REG_SZ,
			nullptr,
			&data,
			len
		);

		if (retCode == ERROR_SUCCESS)
			return WStringToUTF8(std::wstring(data, *len / 2 - 1));

		DWORD number;
		size = sizeof(DWORD);
		len = (LPDWORD)&size;
		retCode = RegGetValueW(
			parent,
			keyName.c_str(),
			keyValName.c_str(),
			RRF_RT_REG_DWORD,
			nullptr,
			&number,
			len
		);
		
		if (retCode == ERROR_SUCCESS)
			return std::to_string(number);

		return "";
	}

	Bitness GetOSBitnessInternal()
	{
#if _WIN64
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
		return IsWine() ? "" : GetStringFromReg(CURRENT_VERSION_KEY, L"ProductName");
	}

	std::string GetOSMajorVersion()
	{
		if (IsWine()) return "";
		std::string version = GetStringFromReg(CURRENT_VERSION_KEY, L"CurrentMajorVersionNumber");
		if (!version.empty()) return version;

		version = GetOSKernelVersion();
		if (version.find('.') != -1) return version.substr(0, version.find('.'));

		return "";
	}

	std::string GetOSMinorVersion()
	{
		if (IsWine()) return "";
		std::string version = GetStringFromReg(CURRENT_VERSION_KEY, L"CurrentMinorVersionNumber");
		if (!version.empty()) return version;

		version = GetOSKernelVersion();
		if (version.find('.') != -1) return version.substr(version.find('.') + 1);

		return "";
	}

	std::string GetOSBuildNumber()
	{
		return IsWine() ? "" : GetStringFromReg(CURRENT_VERSION_KEY, L"CurrentBuild");
	}

	std::string GetOSKernelVersion()
	{
		return IsWine() ? "" : GetStringFromReg(CURRENT_VERSION_KEY, L"CurrentVersion");
	}

	std::string GetHardwareModel()
	{
		auto SystemInformationKey = L"SYSTEM\\CurrentControlSet\\Control\\SystemInformation";

		std::string manu = GetStringFromReg(SystemInformationKey, L"SystemManufacturer");
		if (manu == "System manufacturer") manu = "";

		std::string prod = GetStringFromReg(SystemInformationKey, L"SystemProductName");
		if (prod == "System Product Name") prod = "";
		if (!manu.empty() && !prod.empty()) prod = " [" + prod + "]";

		return manu + prod;
	}

	std::string GetMachineName()
	{
		constexpr auto INFO_BUFFER_SIZE = 1024;
		wchar_t  infoBuf[INFO_BUFFER_SIZE];
		DWORD  bufCharCount = INFO_BUFFER_SIZE;
		return WStringToUTF8(GetComputerNameW(infoBuf, &bufCharCount) ? infoBuf : L"");
	}

	std::string GetUsername()
	{
		wchar_t username[UNLEN + 1];
		DWORD username_len = UNLEN + 1;
		return WStringToUTF8(GetUserNameW(username, &username_len) ? username : L"");
	}

	unsigned GetMonitorCount()
	{
		return GetSystemMetrics(SM_CMONITORS);
	}

	uint64_t GetTotalMemory()
	{
		MEMORYSTATUSEX status;
		status.dwLength = DWORD(sizeof(status));
		return GlobalMemoryStatusEx(&status) ? status.ullTotalPhys : 0;
	}

	std::string GetMachineUuidString()
	{
		REGSAM access = KEY_READ;
#if !_WIN64
		if (GetOSBitnessInternal() != Bitness::BITNESS_32)
			access |= KEY_WOW64_64KEY;
#endif

		HKEY key;
		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", 0, access, &key) != ERROR_SUCCESS) return "";

		DWORD type;
		char guid[256];
		DWORD size = sizeof(guid);
		LSTATUS status = RegQueryValueExA(key, "MachineGuid", nullptr, &type, reinterpret_cast<LPBYTE>(guid), &size);
		RegCloseKey(key);

		return status == ERROR_SUCCESS && type == REG_SZ ? toupper(std::string(guid)) : "";
	}

	std::vector<NetworkAdapterInfo> GetNetworkAdapterInfo()
	{
		if (IsWine()) return {};

		ULONG l = 0;
		DWORD res = GetAdaptersInfo(nullptr, &l);
		if (res != ERROR_BUFFER_OVERFLOW) return {};
		std::vector<char> buf(l);
		res = GetAdaptersInfo( reinterpret_cast<IP_ADAPTER_INFO*>(&buf[0]), &l);
		if (res != ERROR_SUCCESS) return {};

		auto pi = reinterpret_cast<IP_ADAPTER_INFO*>(&buf[0]);
		std::vector<NetworkAdapterInfo> adapters;
		for (; pi; pi = pi->Next)
		{
			std::vector<std::byte> macAddress;
			std::stringstream stream;
			for (unsigned i = 0; i < 6; i++)
			{
				if (i) stream << ":";
				auto val = static_cast<unsigned>(pi->Address[i]);
				if (val <= 0xf) stream << "0";
				stream << std::hex << val;

				macAddress.push_back(static_cast<std::byte>(val));
			}

			std::string uuid = pi->AdapterName;
			if (uuid.rfind("{") == 0 && uuid[uuid.size() - 1] == '}')
				uuid = uuid.substr(1, uuid.size() - 2);

			adapters.push_back
			({
				pi->Description,
				toupper(std::string(stream.str())),
				uuid,
				macAddress
			});
		}

		return adapters;
	}

	std::string GetUserLocale()
	{
		auto locale = std::setlocale(LC_ALL, "");
		std::setlocale(LC_ALL, "C"); // Reset
		return locale ? locale : "";
	}

	bool IsRemoteSession()
	{
		return GetSystemMetrics(SM_REMOTESESSION);
	}

	std::vector<GPUInfo> GetGPUInfo()
	{
		if (IsWine()) return {};
		return GetD3DInfo().adapters;
	}

	std::vector<MonitorInfo> GetMonitorsInfo()
	{
		if (IsWine()) return {};
		return GetD3DInfo().monitors;
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
			return "NONE";
		}
	}

	std::string GetD3DHighestSupport()
	{
		return D3DFeatureSupportToString(GetD3DInfo().maxSupportedFeatureLevel);
	}
}

#else

namespace PDM
{
	std::string GetD3DHighestSupport()
	{
		return "NONE";
	}
}

#endif
