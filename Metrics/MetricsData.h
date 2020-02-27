#pragma once

#include <vector>
#include <string_view>

namespace Metrics
{
	struct DataField
	{
		bool operator ==(const DataField& other) const
		{
			return name == other.name && value == other.value;
		}

		std::string_view name;
		std::string_view value;
	};

	struct MetricsData
	{
		bool operator ==(const MetricsData& other) const
		{
			return items == other.items;
		}

		std::vector<DataField> items;
	};
}
