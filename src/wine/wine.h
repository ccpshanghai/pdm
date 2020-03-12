#pragma once

#include "../../include/pdm.h"

namespace PDM
{
	DllExport bool IsWine();
	DllExport const char* GetWineVersion();
	DllExport const char* GetWineHostOs();
}
