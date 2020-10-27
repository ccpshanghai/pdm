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
	std::string WStringToUTF8(std::wstring_view wideString)
	{
		if (wideString.empty())
		{
			return std::string();
		}
		const int size_needed = WideCharToMultiByte
		(
			CP_UTF8,
			0,                                   // flags
			wideString.data(),                   // from
			static_cast<int>(wideString.size()), // from char count
			nullptr,                             // to
			0,                                   // to byte count
			nullptr,
			nullptr
		);
		std::string result(size_needed, 0);
		WideCharToMultiByte(
			CP_UTF8,
			0,                                   // flags
			wideString.data(),                   // from
			static_cast<int>(wideString.size()), // from char count
			result.data(),                       // to
			static_cast<int>(result.size()),     // to byte count
			nullptr,
			nullptr
		);
		return result;
	}

	std::wstring UTF8ToWString(std::string_view utf8String)
	{
		if (utf8String.empty())
		{
			return std::wstring();
		}
		const int size_needed = MultiByteToWideChar
		(
			CP_UTF8,
			0,                                   // flags
			utf8String.data(),                   // from
			static_cast<int>(utf8String.size()), // from byte count
			nullptr,                             // to
			0                                    // to char count
		);
		std::wstring result(size_needed, 0);
		MultiByteToWideChar
		(
			CP_UTF8,
			0,                                   // flags
			utf8String.data(),                   // from
			static_cast<int>(utf8String.size()), // from byte count
			result.data(),                       // to
			static_cast<int>(result.size())      // to char count
		);
		return result;
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
