#include "utilities.h"
#include "../include/pdm.h"

#include <regex>

#if _WIN32
#include <codecvt>
#include <Windows.h>
#endif

namespace PDM
{
	std::vector<uint8_t> HexStringToByteArray(std::string uuid, size_t byteCount)
	{
		auto str = std::regex_replace(uuid, std::regex("[:-]"), "");
		if (str.length() != byteCount * 2)
			return {};

		std::vector<uint8_t> bytes;
		for (size_t i = 0; i < byteCount; i++)
		{
			try
			{
				int c = std::stoi(str.substr(i*2, 2), nullptr, 16);
				bytes.push_back(c);
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

	std::wstring AnsiToWString(const std::string& str)
	{
		int count = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), nullptr, 0);
		std::wstring wstr(count, 0);
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), &wstr[0], count);
		return wstr;
	}

	bool GetMetalSupported()
	{
		return false;
	}
#endif
}
