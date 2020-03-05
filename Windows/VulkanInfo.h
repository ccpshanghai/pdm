#pragma once

#include <string>

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