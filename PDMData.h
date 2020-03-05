#pragma once

#include <vector>
#include <string_view>
#include <ctime>

namespace PDM
{
	enum class Bitness
	{
		BITNESS_UNKNOWN = 0,
		BITNESS_32 = 32,
		BITNESS_64 = 64
	};

	enum class OS
	{
		WINDOWS,
		MACOS,
		WINE,
		UNKNOWN
	};

	struct TimeStamp : tm
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

	struct DataField
	{
		bool operator ==(const DataField& other) const
		{
			return name == other.name && value == other.value;
		}

		std::string_view name;
		std::string value;
	};

	struct SubItem
	{
		bool operator ==(const SubItem& other) const
		{
			return name == other.name &&
				subitems == other.subitems &&
				items == other.items;
		}

		std::string_view name;
		std::vector<SubItem> subitems;
		std::vector<DataField> items;
	};

	struct PDMData
	{
		bool operator ==(const PDMData& other) const
		{
			return data == other.data && timestamp == other.timestamp;
		}

		SubItem data;
		TimeStamp timestamp;
	};
}
