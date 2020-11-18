#if _WIN32

#include "../defines.h"
#include "../../include/pdm.h"
#include "vulkan/vulkan.h"

#include <string>
#include <windows.h>

namespace PDM
{
	VulkanProperties GetVulkanProperties()
	{
		VulkanProperties properties;

		HMODULE vulkanDll = LoadLibraryA("vulkan-1.dll");
		if (!vulkanDll) return properties;
		SCOPE_EXIT(FreeLibrary(vulkanDll));

		auto vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(GetProcAddress(vulkanDll, "vkCreateInstance"));
		auto vkGetPhysicalDeviceProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(GetProcAddress(vulkanDll, "vkGetPhysicalDeviceProperties"));
		auto vkEnumeratePhysicalDevices = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(GetProcAddress(vulkanDll, "vkEnumeratePhysicalDevices"));
		auto vkDestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(GetProcAddress(vulkanDll, "vkDestroyInstance"));

		if (!vkCreateInstance || !vkGetPhysicalDeviceProperties || !vkEnumeratePhysicalDevices || !vkDestroyInstance) return properties;

		VkInstance instance;
		VkApplicationInfo info{ VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr, "pdm", 0, nullptr, 0, VK_API_VERSION_1_0 };
		VkInstanceCreateInfo vkCreate{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, nullptr, 0, &info, 0, nullptr, 0, nullptr };

		if (vkCreateInstance(&vkCreate, nullptr, &instance) != VK_SUCCESS)
		{
			properties.support = VulkanSupport::UNSUPPORTED;
			return properties;
		}
		SCOPE_EXIT(vkDestroyInstance(instance, nullptr)); // Gets run before FreeLibrary

		properties.support = VulkanSupport::SUPPORTED;

		const unsigned DEVICE_COUNT = 32;
		unsigned count = DEVICE_COUNT;
		VkPhysicalDevice devices[DEVICE_COUNT];

		if (vkEnumeratePhysicalDevices(instance, &count, devices) != VK_SUCCESS) return properties;

		uint32_t maxMajor = 0, maxMinor = 0, maxPatch = 0;
		for (unsigned i = 0; i < count; i++)
		{
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(devices[i], &props);

			uint32_t major = VK_VERSION_MAJOR(props.apiVersion);
			uint32_t minor = VK_VERSION_MINOR(props.apiVersion);
			uint32_t patch = VK_VERSION_PATCH(props.apiVersion);

			if (major > maxMajor ||
				(major == maxMajor && minor > maxMinor) ||
				(major == maxMajor && minor == maxMinor && patch > maxPatch))
			{
				maxMajor = major;
				maxMinor = minor;
				maxPatch = patch;
			}
		}

		properties.version = std::to_string(maxMajor) + "." + std::to_string(maxMinor) + "." + std::to_string(maxPatch);

		return properties;
	}
}

#endif
