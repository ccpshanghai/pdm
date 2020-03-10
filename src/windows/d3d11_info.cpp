#ifdef _WIN32

#include "d3d11_info.h"
#include "../defines.h"

#include <functional>
#include <comdef.h>
#include <dxgi1_6.h>
#include <ShellScalingAPI.h>

#pragma comment(lib, "shcore.lib")

namespace PDM
{
	std::string ws2s(const std::wstring& s)
	{
		auto slength = static_cast<int>(s.length());
		auto len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
		std::string r(len, '\0');
		WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, &r[0], len, 0, 0);
		return r;
	}

	bool GetHexIdFromDeviceId(const char* deviceId, uint32_t& deviceIdHex)
	{
		constexpr auto deviceIdPrefix = "DEV_";

		auto found = strstr(deviceId, deviceIdPrefix);
		if (!found) return false;

		return sscanf_s(found + strlen(deviceIdPrefix), "%x", &deviceIdHex) == 1;
	}

	const char* GetRegistryPathToLocalMachine(const char* registryPath)
	{
		constexpr auto rootPath = "\\Registry\\Machine\\";
		if (strncmp(registryPath, rootPath, strlen(rootPath)) == 0)
			return registryPath + strlen(rootPath);
		else
			return registryPath;
	}

	bool GetDeviceRegistryKey(uint32_t deviceId, std::string& keyPath)
	{
		DISPLAY_DEVICE dd;
		dd.cb = sizeof(DISPLAY_DEVICE);

		for (int i = 0; EnumDisplayDevices(nullptr, i, &dd, 0); ++i)
		{
			uint32_t device;
			if (GetHexIdFromDeviceId(dd.DeviceID, device) && device == deviceId)
			{
				keyPath = GetRegistryPathToLocalMachine(dd.DeviceKey);
				return true;
			}
		}
		return false;
	}

	bool GetRegistryValue(HKEY key, const char* name, std::string& value)
	{
		char buffer[256];
		DWORD dwcb_data = sizeof(buffer);

		if (LONG result = RegQueryValueEx(key, name, nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer), &dwcb_data); result == ERROR_SUCCESS)
		{
			value = buffer;
			return true;
		}
		value = "";
		return false;
	}

	void PopulateAdapterDriverVersion(D3DAdapterInfo& adapter)
	{
		std::string keyPath;
		if (!GetDeviceRegistryKey(adapter.deviceID, keyPath)) return;

		HKEY key;
		if (LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyPath.c_str(), 0, KEY_QUERY_VALUE, &key); result != ERROR_SUCCESS) return;

		GetRegistryValue(key, "DriverVersion", adapter.driverVersionString);
		GetRegistryValue(key, "DriverDate",    adapter.driverDate);
		GetRegistryValue(key, "ProviderName",  adapter.driverVendor);

		RegCloseKey(key);
	}

	////////////////////////////////////////

#pragma warning(disable:26812)
	HRESULT CreateDevice(PFN_D3D11_CREATE_DEVICE d3dCreateDevice, IDXGIAdapter* adapter, D3D_FEATURE_LEVEL& maxSupport)
#pragma warning(default:26812)
	{
		D3D_FEATURE_LEVEL FeatureLevels[] = {
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1
		};

		ID3D11Device* device = nullptr;
		ID3D11DeviceContext* context = nullptr;

		HRESULT hr = E_INVALIDARG;
		__try
		{
			int index = 0;
			while ((hr = d3dCreateDevice(
				adapter,
				D3D_DRIVER_TYPE_UNKNOWN,
				0, 0,
				&FeatureLevels[index++],
				1,
				D3D11_SDK_VERSION,
				&device,
				&maxSupport,
				&context
			)) == E_INVALIDARG)
			{
				if (index >= ARRAYSIZE(FeatureLevels)) break;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			hr = E_FAIL;
		}
		if (context) context->Release();
		if (device) device->Release();

		return hr;
	}

	D3D11Info GetD3DInfo()
	{
		D3D11Info info;

		HMODULE dxgiModuleHandle{};
		HMODULE dx11ModuleHandle{};
		CComPtr<IDXGIFactory> dxgiFactory;

		SCOPE_EXIT
		(
			dxgiFactory = nullptr;
			FreeLibrary(dx11ModuleHandle);
			FreeLibrary(dxgiModuleHandle);
		);

		SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE); // Give us physical monitor resolutions

		dxgiModuleHandle = LoadLibrary("dxgi.dll");
		if (!dxgiModuleHandle) return info;
		dx11ModuleHandle = LoadLibrary("d3d11.dll");
		if (!dx11ModuleHandle) return info;

		typedef HRESULT(WINAPI* LPCreateDXGIFactory)(REFIID riid, IDXGIFactory** ppFactory);
		LPCreateDXGIFactory createDxgiFactory = reinterpret_cast<LPCreateDXGIFactory>(GetProcAddress(dxgiModuleHandle, "CreateDXGIFactory"));
		if (!createDxgiFactory) return info;
		PFN_D3D11_CREATE_DEVICE createDevice = reinterpret_cast<PFN_D3D11_CREATE_DEVICE>(GetProcAddress(dx11ModuleHandle, "D3D11CreateDevice"));
		if (!createDevice) return info;
		if (FAILED(createDxgiFactory(__uuidof(IDXGIFactory), &dxgiFactory.p))) return info;

		uint32_t count = 0;
		IDXGIAdapter* pAdapter;

		HMONITOR primaryMonitor = MonitorFromPoint({ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);

		while (dxgiFactory->EnumAdapters(count++, reinterpret_cast<IDXGIAdapter**>(&pAdapter)) != DXGI_ERROR_NOT_FOUND)
		{
			D3D_FEATURE_LEVEL support = info.maxSupportedFeatureLevel;

			if (FAILED(CreateDevice(createDevice, pAdapter, support))) continue;

			if (support > info.maxSupportedFeatureLevel)
				info.maxSupportedFeatureLevel = support;

			uint32_t index = 0;
			CComPtr<IDXGIOutput> pOutput;
			while (SUCCEEDED(pAdapter->EnumOutputs(index, reinterpret_cast<IDXGIOutput**>(&pOutput))))
			{
				DXGI_OUTPUT_DESC outpDesc;
				pOutput->GetDesc(&outpDesc);

				uint32_t width = outpDesc.DesktopCoordinates.right - outpDesc.DesktopCoordinates.left;
				uint32_t height = outpDesc.DesktopCoordinates.bottom - outpDesc.DesktopCoordinates.top;

				if (outpDesc.Rotation == DXGI_MODE_ROTATION_ROTATE90 || outpDesc.Rotation == DXGI_MODE_ROTATION_ROTATE270)
					std::swap(width, height);

				uint32_t bpc = 0;
				if (CComQIPtr<IDXGIOutput6> pOutput6(pOutput); pOutput6)
				{
					pOutput = nullptr; // Need to do this explicitly
					DXGI_OUTPUT_DESC1 outpDesc1;
					pOutput6->GetDesc1(&outpDesc1);
					bpc = outpDesc1.BitsPerColor;
				}

				MonitorInfo monitor{ width, height, bpc };

				bool isPrimary = outpDesc.Monitor == primaryMonitor;
				if (isPrimary) // Always have primary monitor first
					info.monitors.insert(info.monitors.begin(), monitor);
				else
					info.monitors.push_back(monitor);

				index++;
			}

			DXGI_ADAPTER_DESC desc{ 0 };
			pAdapter->GetDesc(&desc);

			std::string description(ws2s(desc.Description));
			if (description == "Microsoft Basic Render Driver") continue;

			D3DAdapterInfo adapter;
			adapter.description = description;
			adapter.vendorID = desc.VendorId;
			adapter.deviceID = desc.DeviceId;
			adapter.subSystemID = desc.SubSysId;
			adapter.revision = desc.Revision;
			PopulateAdapterDriverVersion(adapter);

			info.adapters.push_back(adapter);
		}

		return info;
	}
}

#endif
