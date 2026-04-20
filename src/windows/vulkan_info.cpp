// Copyright © 2026 CCP ehf.

#if _WIN32

#include "../defines.h"
#include "../../include/pdm.h"

#include <string>
#include <windows.h>

namespace PDM
{
	// Stubbed due to causing crashes
	VulkanProperties GetVulkanProperties()
	{
		return {};
	}
}

#endif
