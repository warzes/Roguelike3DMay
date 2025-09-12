#pragma once

namespace io
{
	bool Exists(const std::string& filePath);

	std::filesystem::path CurrentPath();

	std::string GetFileExtension(const std::string& filePath);
	std::string GetFileName(const std::string& filePath);
	std::string GetFileNameWithoutExtension(const std::string& filePath);
	std::string GetFileDirectory(const std::string& filePath);

	std::string LoadFile(const std::filesystem::path& path);
	std::vector<char> LoadBinaryFile(const std::filesystem::path& path);

	std::string ReadShaderCode(const std::string& filename, const std::vector<std::string>& defines = {});
} // namespace io