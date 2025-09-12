#pragma once

namespace str
{
	// See https://stackoverflow.com/questions/5878775/how-to-find-and-replace-string
	inline bool ReplaceFirst(std::string& src, const std::string& oldstr, const std::string& newstr)
	{
		const std::size_t pos = src.find(oldstr);
		if (pos == std::string::npos)
		{
			return false;
		}
		src.replace(pos, oldstr.size(), newstr);
		return true;
	}

	inline bool ReplaceAll(std::string& src, const std::string& oldstr, const std::string& newstr)
	{
		size_t pos = 0;
		bool replaced = false;
		while ((pos = src.find(oldstr, pos)) != std::string::npos)
		{
			replaced = true;
			src.replace(pos, oldstr.size(), newstr);
			pos += newstr.length();
		}

		return replaced;
	}

	inline bool BeginsWith(const std::string& src, const std::string& phrase)
	{
		if (phrase.size() > src.size()) return false;

		for (size_t i = 0; i < phrase.size(); ++i)
		{
			if (src[i] != phrase[i]) return false;
		}

		return true;
	}

} // namespace str