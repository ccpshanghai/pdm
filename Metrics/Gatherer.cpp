#include "Gatherer.h"

namespace Metrics
{
	const MetricsData GatherData() throw()
	{
		return MetricsData{ { {"name1", "test1"}, {"name2", "test3"} } };
	}
}