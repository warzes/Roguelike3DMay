#pragma once

namespace StrUtils
{
	inline char ToLowercase(char& character)
	{
		character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
		return character;
	}

	inline wchar_t ToLowercase(wchar_t& character)
	{
		character = static_cast<wchar_t>(std::towlower(character));
		return character;
	}

	inline std::string& ToLowercase(std::string& text) 
	{
		std::transform(text.begin(), text.end(), text.begin(), [](char character) { return ToLowercase(character); });
		return text;
	}

	inline std::wstring& ToLowercase(std::wstring& text)
	{
		std::transform(text.begin(), text.end(), text.begin(), [](wchar_t character) { return ToLowercase(character); });
		return text;
	}

	inline char ToUppercase(char& character)
	{
		character = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
		return character;
	}

	inline wchar_t ToUppercase(wchar_t& character)
	{
		character = static_cast<wchar_t>(std::towupper(character));
		return character;
	}

	inline std::string& ToUppercase(std::string& text)
	{
		std::transform(text.begin(), text.end(), text.begin(), [](char character) { return ToUppercase(character); });
		return text;
	}

	inline std::wstring& ToUppercase(std::wstring& text)
	{
		std::transform(text.begin(), text.end(), text.begin(), [](wchar_t character) { return ToUppercase(character); });
		return text;
	}

	inline std::string& TrimLeft(std::string& text)
	{
		text.erase(text.begin(), std::find_if_not(text.begin(), text.end(), [](unsigned char c) {
			return std::isspace(c);
			}));
		return text;
	}

	inline std::wstring& TrimLeft(std::wstring& text)
	{
		text.erase(text.begin(), std::find_if_not(text.begin(), text.end(), [](wchar_t c) {
			return std::iswspace(c);
			}));
		return text;
	}

	inline std::string& TrimRight(std::string& text)
	{
		text.erase(std::find_if_not(text.rbegin(), text.rend(), [](unsigned char c) {
			return std::isspace(c);
			}).base(), text.end());
		return text;
	}

	inline std::wstring& TrimRight(std::wstring& text)
	{
		text.erase(std::find_if_not(text.rbegin(), text.rend(), [](wchar_t c) {
			return std::iswspace(c);
			}).base(), text.end());
		return text;
	}

	inline std::string& Trim(std::string& text)
	{
		TrimLeft(text);
		TrimRight(text);
		return text;
	}

	inline std::wstring& Trim(std::wstring& text)
	{
		TrimLeft(text);
		TrimRight(text);
		return text;
	}

	inline std::vector<std::string> Split(std::string text, char delimiter)
	{
		TrimRight(text);

		std::vector<std::string> parts{};

		while (!text.empty())
		{
			const std::size_t delimPos = text.find_first_of(delimiter);

			if (delimPos > text.size()) 
			{
				parts.emplace_back(std::move(text));
				break;
			}

			parts.emplace_back(text.substr(0, delimPos));
			TrimRight(parts.back());

			text.erase(0, delimPos + 1);
			TrimLeft(text);
		}

		return parts;
	}

	inline std::vector<std::wstring> Split(std::wstring text, wchar_t delimiter)
	{
		TrimRight(text);

		std::vector<std::wstring> parts{};

		while (!text.empty())
		{
			const std::size_t delimPos = text.find_first_of(delimiter);

			if (delimPos > text.size())
			{
				parts.emplace_back(std::move(text));
				break;
			}

			parts.emplace_back(text.substr(0, delimPos));
			TrimRight(parts.back());

			text.erase(0, delimPos + 1);
			TrimLeft(text);
		}

		return parts;
	}

} // namespace StrUtils