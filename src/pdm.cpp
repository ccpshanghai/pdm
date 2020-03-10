#include "../include/pdm.h"
#include "gatherer.h"

namespace PDM
{
	const PDMData& RetrievePDMData()
	{
		static PDMData data{};
		static bool hasData = false;

		if (!hasData)
		{
			data = GatherData();
			hasData = true;
		}

		return data;
	}
}
