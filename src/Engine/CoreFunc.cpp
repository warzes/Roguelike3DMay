#include "stdafx.h"
#include "CoreFunc.h"
#include "Log.h"
//=============================================================================
std::string GetFileExtension(const std::string& filePath)
{
	std::filesystem::path path(filePath);
	return path.extension().string();
}
//=============================================================================
std::string GetFileName(const std::string& filePath)
{
	std::filesystem::path path(filePath);
	return path.filename().string();
}
//=============================================================================
std::string GetFileNameWithoutExtension(const std::string& filePath)
{
	std::filesystem::path path(filePath);
	std::string fileName = GetFileName(filePath);
	std::string extension = GetFileExtension(filePath);

	if (!extension.empty())
		return fileName.substr(0, fileName.size() - extension.size());

	return fileName;
}
//=============================================================================
std::string GetFileDirectory(const std::string& filePath)
{
	std::filesystem::path path(filePath);
	return path.parent_path().string() + "/";
}
//=============================================================================
std::string ReadTextFile(const std::string& filename)
{
	struct FileRaw
	{
		~FileRaw() { if (file) fclose(file); }
		FILE* file{ nullptr };
	} file;

	errno_t err = fopen_s(&file.file, filename.c_str(), "rb");
	if (err != 0 || file.file == nullptr)
	{
		Error("Unable to open file '" + filename + "'.");
		return "";
	}

	if (fseek(file.file, 0, SEEK_END) != 0)
	{
		Error("Unable to move pointer to the end of file '" + filename + "'.");
		return "";
	}

	const long fileSize = ftell(file.file);
	if (fileSize < 0)
	{
		Error("Unable to get the size of file  '" + filename + "'.");
		return "";
	}

	if (fseek(file.file, 0, SEEK_SET) != 0)
	{
		Error("Unable to return pointer to the beginning of file '" + filename + "'.");
		return "";
	}

	std::string content(static_cast<size_t>(fileSize), '\0');

	const size_t bytesRead = fread_s(content.data(), static_cast<size_t>(fileSize), 1, static_cast<size_t>(fileSize), file.file);
	if (bytesRead != static_cast<size_t>(fileSize))
	{
		Error("Unable to read all data from file '" + filename + "'.");
		return "";
	}

	fclose(file.file);

	return content;
}
//=============================================================================
std::string headerGuardFromPath(const std::string& path)
{
	std::string out = GetFileNameWithoutExtension(path);
	std::transform(out.begin(), out.end(), out.begin(), ::toupper);
	return out + "_H";
}
//=============================================================================
bool preprocessShader(const std::string& path, const std::string& src, std::string& out)
{
	std::istringstream       stream(src);
	std::string              line;
	std::vector<std::string> includedHeaders;

	while (std::getline(stream, line))
	{
		if (line.find("#include") != std::string::npos)
		{
			size_t      start = line.find_first_of("<") + 1;
			size_t      end = line.find_last_of(">");
			std::string includePath = line.substr(start, end - start);

			std::string pathToShader = "";
			size_t      slashPos = path.find_last_of("/");

			if (slashPos != std::string::npos)
				pathToShader = path.substr(0, slashPos + 1);

			std::string includeSource;

			std::string source = ReadTextFile(pathToShader + includePath);
			if (source.empty()) return false;

			if (!preprocessShader(pathToShader + includePath, source, includeSource))
			{
				Error("Included file <" + includePath + "> cannot be opened!");
				return false;
			}
			if (Contains(includedHeaders, includePath))
				Warning("Header <" + includePath + "> has been included twice!");
			else
			{
				includedHeaders.push_back(includePath);

				std::string headerGuard = headerGuardFromPath(includePath);

				out += "#ifndef ";
				out += headerGuard;
				out += "\n#define ";
				out += headerGuard;
				out += "\n\n";
				out += includeSource + "\n\n";
				out += "#endif\n\n";
			}
		}
		else
			out += line + "\n";
	}

	return true;
}
//=============================================================================
std::string ReadShaderCode(const std::string& filename, const std::vector<std::string>& defines)
{
	std::string source = ReadTextFile(filename);
	if (source.empty()) return "";

	std::string finalSource;

	if (defines.size() > 0)
	{
		for (auto define : defines)
			finalSource += "#define " + define + "\n";

		finalSource += "\n";
	}

	if (!preprocessShader(filename, source, finalSource)) return "";
	return finalSource;
}
//=============================================================================