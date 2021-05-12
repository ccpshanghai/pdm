#pragma once

#include <string>
#include <vector>
#include "../include/pdm_data.h"

namespace PDM
{
	std::vector<uint8_t> HexStringToByteArray(std::string uuid, size_t byteCount);
	std::string tolower(std::string str);

#if _WIN32
	std::string WStringToUTF8(const std::wstring_view wideString);
	std::string WStringToNative(const std::wstring_view wideString);
	std::wstring NativeToWString(const std::string_view str);
#endif
}
