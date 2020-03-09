#pragma once

#include <string>

namespace PDM
{
	enum class VulkanSupport
	{
		UNKNOWN,
		SUPPORTED,
		UNSUPPORTED,
	};

	struct VulkanProperties
	{
		VulkanSupport support{ VulkanSupport::UNKNOWN };
		std::string version;
	};

	VulkanProperties GetVulkanProperties();
}