#pragma once

#ifdef PDM_DLL_EXPORT

#if _WIN32
#define PDMDllExport __declspec( dllexport )
#else
#define PDMDllExport __attribute__(( visibility( "default" ) ))
#endif

#else

#define PDMDllExport

#endif

#include <vector>
#include <string>
#include <ctime>

namespace PDM
{
	enum class Bitness
	{
		BITNESS_UNKNOWN =  0,
		BITNESS_32      = 32,
		BITNESS_64      = 64
	};

	enum class OS
	{
		UNKNOWN = 0,
		WINDOWS,
		MACOS,
		WINE,
	};

	enum class StreamingService
	{
		NONE = 0,
		UNKNOWN,
		INTEL,
	};

	enum class VulkanSupport
	{
		UNKNOWN = 0,
		SUPPORTED,
		UNSUPPORTED,
	};

	struct PDMDllExport CPUInfo
	{
		int model{ 0 };
		int stepping{ 0 };
		std::string vendor;
		std::string brand;
		Bitness bitness;
		unsigned logicalCoreCount{ 0 };
	};

	struct PDMDllExport MonitorInfo
	{
		std::string name;
		uint32_t width{};
		uint32_t height{};
		uint32_t bitsPerColor{};
		uint32_t refreshRate{};
		uint32_t dpiScaling{};
	};

	struct PDMDllExport GPUInfo
	{
		std::string description;
		uint32_t vendorID{};
		uint32_t deviceID{};
		uint32_t revision{};
		uint64_t memory{};
		std::string driverVersionString;
		std::string driverDate;
		std::string driverVendor;
	};

	struct PDMDllExport NetworkAdapterInfo
	{
		std::string name;
		std::string macAddressString;
		std::string uuidString;
		std::vector<uint8_t> macAddress;
		std::vector<uint8_t> uuid;
	};

	struct PDMDllExport VulkanProperties
	{
		VulkanSupport support{ VulkanSupport::UNKNOWN };
		std::string version;
	};

	struct PDMDllExport TimeStamp : tm
	{
		bool operator ==(const TimeStamp& other) const
		{
			return
				tm_sec   == other.tm_sec  &&
				tm_min   == other.tm_min  &&
				tm_hour  == other.tm_hour &&
				tm_mday  == other.tm_mday &&
				tm_mon   == other.tm_mon  &&
				tm_year  == other.tm_year &&
				tm_yday  == other.tm_yday &&
				tm_isdst == other.tm_isdst;
		}
	};

	struct PDMDllExport DataField
	{
		bool operator ==(const DataField& other) const
		{
			return name == other.name && value == other.value;
		}

		std::string name;
		std::string value;
	};

	struct PDMDllExport SubItem
	{
		bool operator ==(const SubItem& other) const
		{
			return name == other.name &&
				subitems == other.subitems &&
				items == other.items;
		}

		std::string name;
		std::vector<SubItem> subitems;
		std::vector<DataField> items;
	};

	struct PDMDllExport PDMData
	{
		bool operator ==(const PDMData& other) const
		{
			return data == other.data && timestamp == other.timestamp;
		}

		SubItem data;
		TimeStamp timestamp;
	};
}
