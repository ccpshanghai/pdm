#pragma once

#include <vector>
#include <string_view>

namespace Metrics
{
	struct DataField
	{
		bool DataField::operator ==(const DataField& b) const = default;

		std::string_view name;
		std::string_view value;
	};

	struct MetricsData
	{
		bool MetricsData::operator ==(const MetricsData& b) const = default;

		std::vector<DataField> items;
	};
}