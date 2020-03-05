#pragma once

#ifdef _WIN32

#include "../Metrics.h"

#include <string>

namespace Metrics
{
	struct AdapterInfo
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

	Metrics::SubItem GetWindowsSubItems();
}

#endif