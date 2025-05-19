#pragma once

namespace FileUtils
{
	std::string GetFileExtension(const std::string& filePath);
	std::string GetFileName(const std::string& filePath);
	std::string GetFileNameWithoutExtension(const std::string& filePath);
	std::string GetFileDirectory(const std::string& filePath);

	std::string ReadTextFile(const std::string& filename);
	std::string ReadShaderCode(const std::string& filename, const std::vector<std::string>& defines = {});
} // namespace FileUtils