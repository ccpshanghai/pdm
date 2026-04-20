// Copyright © 2026 CCP ehf.

#pragma once

#if _WIN32

#include "../../include/pdm.h"

#include <d3d11.h>
#include <d3d12.h>
#include <atlcomcli.h>
#include <dxgi.h>
#include <string>
#include <vector>

namespace PDM
{
	const auto D3D_FEATURE_LEVEL_12_2     = static_cast<D3D_FEATURE_LEVEL>(0xc200);
	const auto D3D_FEATURE_LEVEL_1_0_CORE = static_cast<D3D_FEATURE_LEVEL>(0x1000);
	const auto D3D_FEATURE_LEVEL_NONE     = static_cast<D3D_FEATURE_LEVEL>(0x0);

	struct D3D11Info
	{
		D3D_FEATURE_LEVEL maxSupportedFeatureLevel{ D3D_FEATURE_LEVEL_NONE };
		std::vector<GPUInfo> adapters;
		std::vector<MonitorInfo> monitors;
	};

	D3D11Info& GetD3DInfo();
}

#endif
