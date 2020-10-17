#pragma once

#include <string>
#include <vector>
#include "../include/pdm_data.h"

namespace PDM
{
	std::vector<std::byte> HexStringToByteArray(std::string uuid, size_t byteCount);

#if _WIN32
	std::string WStringToUTF8(const wchar_t* string);
	std::string WStringToUTF8(const std::wstring& string);
	std::wstring UTF8ToWString(const std::string& utf8);
#endif
}
