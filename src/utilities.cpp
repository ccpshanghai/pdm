#include "utilities.h"
#include "../include/pdm.h"

#include <regex>
#include <codecvt>

namespace PDM
{
	std::vector<std::byte> HexStringToByteArray(std::string uuid, size_t byteCount)
	{
		auto str = std::regex_replace(uuid, std::regex("[:-]"), "");
		if (str.length() != byteCount * 2)
			return {};

		std::vector<std::byte> bytes;
		for (size_t i = 0; i < byteCount; i++)
		{
			try
			{
				int c = std::stoi(str.substr(i*2, 2), nullptr, 16);
				bytes.push_back(static_cast<std::byte>(c));
			}
			catch (std::invalid_argument&)
			{
				return {};
			}
		}

		return bytes;
	}

#if _WIN32
	std::string WStringToUTF8(const wchar_t* string)
	{
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(string);
	}

	std::string WStringToUTF8(const std::wstring& string)
	{
		return WStringToUTF8(string.c_str());
	}

	std::wstring UTF8ToWString(const std::string& utf8)
	{
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(utf8);
	}

	bool GetMetalSupported()
	{
		return false;
	}
#endif
}
