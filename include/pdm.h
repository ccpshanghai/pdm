#pragma once

#include "pdm_data.h"

#ifdef _WIN32
#define DllExport __declspec( dllexport )
#else
#define DllExport __attribute__((visibility("default")))
#endif

namespace PDM
{
	DllExport const PDMData& RetrievePDMData();

	OS GetOSType();
	std::string GetOSName();
	std::string GetOSMajorVersion();
	std::string GetOSMinorVersion();
	std::string GetOSBuildNumber();
	std::string GetOSKernelVersion();
	std::string GetMachineName();
	std::string GetUsername();
	unsigned GetScreenCount();
	uint64_t GetTotalMemory();
	std::string GetMachineUuid();
}
