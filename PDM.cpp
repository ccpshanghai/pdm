#include "PDM.h"
#include "Gatherer.h"

namespace PDM
{
	const PDMData& RetrievePDMData()
	{
		static PDMData s_data{};
		static bool s_hasData = false;

		if (!s_hasData)
		{
			s_data = GatherData();
			s_hasData = true;
		}

		return s_data;
	}
}