#if _WIN32
#include <atlcomcli.h>
#endif

#include "utilities.h"
#include "../include/pdm.h"

#include <regex>
#include <codecvt>

namespace PDM
{
	std::vector<std::byte> HexStringToByteArray(UTF8String uuid, size_t byteCount)
	{
		auto str = std::regex_replace(uuid.GetUTF8String(), std::regex("[:-]"), "");
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

	UTF8String::UTF8String() {}

	UTF8String::UTF8String(const std::string& string)
	{
		_utf8String = string;
	}

	UTF8String::UTF8String(const char* string)
	{
		_utf8String = std::string(string);
	}

	std::string UTF8String::GetUTF8String() const
	{
		return _utf8String;
	}

	bool UTF8String::operator ==(const UTF8String& other) const
	{
		return _utf8String == other._utf8String;
	}

	bool UTF8String::operator !=(const UTF8String& other) const
	{
		return _utf8String != other._utf8String;
	}

	UTF8String UTF8String::operator +(const UTF8String& other) const
	{
		return UTF8String(_utf8String + other._utf8String);
	}

	size_t UTF8String::length() const
	{
		return _utf8String.length();
	}

	bool UTF8String::empty() const
	{
		return _utf8String.empty();
	}

#if _WIN32
	UTF8String::UTF8String(const std::wstring& string)
	{
		_utf8String = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(string);
	}

	UTF8String::UTF8String(const wchar_t* string)
	{
		_utf8String = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(string);
	}

	UTF8String::operator std::wstring()
	{
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(_utf8String);
	}

	std::wstring UTF8String::GetNativeString() const
	{
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(_utf8String);
	}

	bool GetMetalSupported()
	{
		return false;
	}
#else
	std::string UTF8String::GetNativeString() const
	{
		return _utf8String;
	}
#endif
}
