#pragma once

#include "MetricsData.h"

#ifdef _WIN32
	#define DllExport __declspec( dllexport )
#else
	#define DllExport __attribute__((visibility("default")))
#endif

namespace Metrics
{
	DllExport MetricsData& RetrieveMetricsData();
}