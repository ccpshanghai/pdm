#include "Metrics_CLI.h"
#include "Metrics.h"

#include <iostream>

int main()
{
	auto& metrics = Metrics::RetrieveMetricsData();
	for (auto& data : metrics.items)
	{
		std::cout << data.name << ": " << data.value << std::endl;
	}
	return 0;
}
