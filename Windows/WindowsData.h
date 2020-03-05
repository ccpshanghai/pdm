#pragma once

#ifdef _WIN32

#include "../PDM.h"

#include <string>

namespace PDM
{
	struct NetworkAdapterInfo
	{
		std::string name;
		std::string description;
		std::string macAddress;
	};

	std::string GetWindowsName();
	std::string GetWindowsMajorVersion();
	std::string GetWindowsMinorVersion();
	std::string GetWindowsBuildNumber();
	std::string GetWindowsKernelVersion();

	SubItem GetWindowsSubItems();
}

#endif