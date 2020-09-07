#pragma once

#if _WIN32

#include <string>

namespace PDM
{
	std::string ws2s(const std::wstring& s);
}

#endif
