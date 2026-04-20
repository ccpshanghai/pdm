// Copyright © 2026 CCP ehf.

#include "output.h"
#include "utilities.h"

#include <fstream>

#if _WIN32

#include <windows.h>
#include <io.h>
#include <fcntl.h>

std::wstring UTF8ToWStringOrString(const std::string& str)
{
	return PDM::UTF8ToWString(str);
}

#else

std::string UTF8ToWStringOrString(const std::string& str)
{
	return str;
}

#endif

using namespace PDM;

std::string TimestampToString(const TimeStamp& timestamp)
{
	const int MAX_SIZE = 20;
	char time[MAX_SIZE];
	strftime(time, MAX_SIZE, "%Y_%m_%d_%H_%M_%S", &timestamp);
	return time;
}

void OutputExecutionTimings(std::string filename)
{
	std::fstream outfile;
	outfile.open(UTF8ToWStringOrString(filename), std::ios::out);

	if (outfile)
	{
		const unsigned threshold = 100;
		const unsigned runs = 100;

		outfile << "Runs: " << runs << " Threshold: " << threshold << "\n";
		
		uint64_t average = 0;
		unsigned crossings = 0;

		for (unsigned i = 0; i < runs; i++)
		{
			uint64_t cycles = GetTimingCycles();
			average += cycles;

			outfile << "Cycles: ";
			if (cycles > threshold)
			{
				outfile << "*";
				crossings++;
			}
			outfile << cycles << "\n";
		}

		average /= runs;

		outfile << "Average: " << average << " Crossings: " << crossings << std::endl;
		outfile.close();
	}
}

auto Execute()
{
	try
	{
#if _WIN32
		SetConsoleOutputCP(CP_UTF8);
		SetConsoleCP(CP_UTF8);
#endif
		const auto& data = RetrievePDMData("pdmCLI", "1.0");

		std::fstream outfile;
		std::string filename = "PDM_Output_" + GetMachineName() + "_" + TimestampToString(data.timestamp);

		outfile.open(UTF8ToWStringOrString(filename + ".txt"), std::ios::out);

		if (!outfile)
		{
			std::cout << "Error: Could not open output file!\n\n"; // But dump to console anyway
		}
		else
		{
			Output(data, outfile);
			outfile.close();
		}

		Output(data, std::cout);

		OutputExecutionTimings(filename + "_VM_Execution_Timings.txt");
	}
	catch (std::exception& e)
	{
		std::cout << "Unexpected exception: " << e.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void Wait()
{
	std::cout << "Press Enter to exit...";
	std::cin.get();
}

int main()
{
	auto code = Execute();
	Wait();
	return code;
}
