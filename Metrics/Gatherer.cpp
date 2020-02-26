#include "Gatherer.h"

namespace Metrics
{
	MetricsData GatherData()
	{
		return MetricsData{ { {"name1", "test1"}, {"name2", "test2"} } };
	}
}