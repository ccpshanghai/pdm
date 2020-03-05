#pragma once

#include "PDMData.h"

#ifdef _WIN32
#define DllExport __declspec( dllexport )
#else
#define DllExport __attribute__((visibility("default")))
#endif

namespace PDM
{
	DllExport const PDMData& RetrievePDMData();

	OS GetOSType();
	const std::string GetOSName();
	const std::string GetOSMajorVersion();
	const std::string GetOSMinorVersion();
	const std::string GetOSBuildNumber();
	const std::string GetOSKernelVersion();
	const std::string GetMachineName();
	const std::string GetUsername();
	unsigned GetScreenCount();
	uint64_t GetTotalMemory();
	const std::string GetMachineUuid();
	bool IsRunningVM();
}