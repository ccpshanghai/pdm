#pragma once

#include <string>
#include <vector>
#include "../include/pdm_data.h"

namespace PDM
{
	std::vector<std::byte> HexStringToByteArray(UTF8String uuid, size_t byteCount);
}
