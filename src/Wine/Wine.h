#pragma once

#include "../../include/PDM.h"

namespace PDM
{
	DllExport bool IsWine();
	DllExport const char* GetWineVersion();
	DllExport const char* GetWineHostOs();

	SubItem GetWineSubItems();
}