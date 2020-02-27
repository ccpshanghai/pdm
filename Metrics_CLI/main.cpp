#include "Metrics.h"

#include <iostream>
#include <algorithm>
#include <fstream>

using namespace Metrics;

void Wait()
{
    std::cout << "Press Enter to exit...";
    std::cin.get();
}

void OutputLines(const MetricsData metrics, std::ostream& stream)
{
	std::for_each(cbegin(metrics.items), cend(metrics.items), [&stream](const DataField& item)
		{
			stream << item.name << ": " << item.value << '\n';
		});

	stream.flush();
}

auto Execute()
{
	try
	{
		const auto& metrics = RetrieveMetricsData();

		std::fstream outfile;
		outfile.open("MetricsOutput.txt", std::ios::out);

		if (!outfile)
		{
			std::cout << "Could not open output file!\n"; // But dump to console anyway
		}
		else
		{
			OutputLines(metrics, outfile);
			outfile.close();
		}

		OutputLines(metrics, std::cout);
	}
	catch (std::exception & e)
	{
		std::cout << "Unexpected exception: " << e.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int main()
{
	auto code = Execute();
	Wait();
	return code;
}
