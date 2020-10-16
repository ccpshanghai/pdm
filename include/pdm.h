#pragma once

#include "pdm_data.h"

namespace PDM
{
	DllExport const PDMData& RetrievePDMData(std::string applicationName, std::string applicationVersion);

	DllExport UTF8String GetPDMVersion();
	DllExport OS GetOSType();
	DllExport UTF8String GetOSName();
	DllExport UTF8String GetOSMajorVersion();
	DllExport UTF8String GetOSMinorVersion();
	DllExport UTF8String GetOSBuildNumber();
	DllExport UTF8String GetOSKernelVersion();
	DllExport UTF8String GetHardwareModel();
	DllExport UTF8String GetMachineName();
	DllExport UTF8String GetUsername();
	DllExport UTF8String GetUserLocale();
	DllExport unsigned GetMonitorCount();
	DllExport uint64_t GetTotalMemory();
	DllExport bool IsRemoteSession();
	DllExport size_t GetTimingCycles();
	DllExport UTF8String GetMachineUuidString();
	DllExport std::vector<std::byte> GetMachineUuid();
	DllExport std::vector<MonitorInfo> GetMonitorsInfo();
	DllExport std::vector<GPUInfo> GetGPUInfo();
	DllExport std::vector<NetworkAdapterInfo> GetNetworkAdapterInfo();
	DllExport bool GetMetalSupported();
	DllExport VulkanProperties GetVulkanProperties();
	DllExport UTF8String GetD3DHighestSupport();
	DllExport bool IsWine();
	DllExport const char* GetWineVersion();
	DllExport const char* GetWineHostOs();
	DllExport Bitness GetProcessBitness();
	DllExport Bitness GetOSBitness();
	DllExport CPUInfo GetCPUInfo();
	DllExport bool HasVMExecutionTiming();
	DllExport bool HasHypervisorBit();
	DllExport UTF8String GetHypervisorName();
	DllExport bool IsHyperVGuestOS();
	DllExport bool IsSuspectedVM();
}
