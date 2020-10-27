#pragma once

#include <string>
#include <vector>
#include "../include/pdm_data.h"

namespace PDM
{
	std::vector<uint8_t> HexStringToByteArray(std::string uuid, size_t byteCount);

#if _WIN32
	std::string WStringToUTF8(std::wstring_view wideString);
	std::wstring UTF8ToWString(std::string_view utf8String);
	std::wstring AnsiToWString(const std::string& str);
#endif
}
