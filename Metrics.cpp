#include "Metrics.h"
#include "Gatherer.h"

namespace Metrics
{
	const MetricsData& RetrieveMetricsData() throw()
	{
		static MetricsData s_data{};
		static bool s_hasData = false;

		if (!s_hasData)
		{
			s_data = GatherData();
			s_hasData = true;
		}

		return s_data;
	}
}