#pragma once

#include "pdm_data.h"

#if _WIN32
#define DllExport __declspec( dllexport )
#else
#define DllExport __attribute__((visibility("default")))
#endif

namespace PDM
{
	DllExport const PDMData& RetrievePDMData();

	OS GetOSType();
	std::string GetOSName();
	unsigned GetOSMajorVersion();
	unsigned GetOSMinorVersion();
	unsigned GetOSBuildNumber();
	std::string GetOSKernelVersion();
	std::string GetHardwareModel();
	std::string GetMachineName();
	std::string GetUsername();
	std::string GetUserLocale();
	unsigned GetMonitorCount();
	uint64_t GetTotalMemory();
	bool IsRemoteSession();
	std::string GetMachineUuid();
	std::vector<MonitorInfo> GetMonitorsInfo();
	std::vector<GPUInfo> GetGPUInfo();
	std::vector<NetworkAdapterInfo> GetNetworkAdapterInfo();
	bool GetMetalSupported();
	VulkanProperties GetVulkanProperties();
	std::string GetD3DHighestSupport();
	bool IsWine();
	const char* GetWineVersion();
	const char* GetWineHostOs();
}
