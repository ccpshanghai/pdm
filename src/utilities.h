#pragma once

#include <string>
#include <vector>

namespace PDM
{
	std::vector<std::byte> HexStringToByteArray(std::string uuid, size_t byteCount);

#if _WIN32
	std::string ws2s(const std::wstring& s);
#endif
}
