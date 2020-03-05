#pragma once

#include "../Metrics.h"

namespace Wine
{
	DllExport bool IsWine();
	DllExport const char* GetWineVersion();
	DllExport const char* GetWineHostOs();

	Metrics::SubItem GetWineSubItems();
}