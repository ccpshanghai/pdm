#include "utilities.h"
#include "../include/pdm.h"

#include <regex>

namespace PDM
{
	std::vector<std::byte> HexStringToByteArray(std::string uuid, size_t byteCount)
	{
		uuid = std::regex_replace(uuid, std::regex("[:-]"), "");
		if (uuid.length() != byteCount * 2)
			return {};

		std::vector<std::byte> bytes;
		for (size_t i = 0; i < byteCount; i++)
		{
			try
			{
				int c = std::stoi(uuid.substr(i*2, 2), nullptr, 16);
				bytes.push_back(static_cast<std::byte>(c));
			}
			catch (std::invalid_argument&)
			{
				return {};
			}
		}

		return bytes;
	}
}

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