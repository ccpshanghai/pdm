#pragma once

#if _WIN32

#include <d3d11.h>
#include <atlcomcli.h>
#include <dxgi.h>
#include <string>
#include <vector>

namespace PDM
{
	struct D3DAdapterInfo
	{
		std::string description;
		uint32_t vendorID{};
		uint32_t deviceID{};
		uint32_t subSystemID{};
		uint32_t revision{};
		std::string driverVersionString;
		std::string driverDate;
		std::string driverVendor;
	};

	struct MonitorInfo
	{
		uint32_t width{};
		uint32_t height{};
		uint32_t bitsPerColor{};
	};

	struct D3D11Info
	{
		D3D_FEATURE_LEVEL maxSupportedFeatureLevel{ static_cast<D3D_FEATURE_LEVEL>(0) };
		std::vector<D3DAdapterInfo> adapters;
		std::vector<MonitorInfo> monitors;
	};

	D3D11Info GetD3DInfo();
}

#endif