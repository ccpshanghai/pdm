#include "utilities.h"
#include "../include/pdm.h"

#if _WIN32

#include <atlcomcli.h>

namespace PDM
{
	std::string ws2s(const std::wstring& s)
	{
		auto slength = static_cast<int>(s.length());
		auto len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, nullptr, 0, nullptr, nullptr);
		std::string r(len, '\0');
		WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, &r[0], len, nullptr, nullptr);
		return r;
	}

	bool GetMetalSupported()
	{
		return false;
	}
}

#endif