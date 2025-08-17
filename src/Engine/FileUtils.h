#pragma once

namespace io
{
	bool Exists(const std::string& filePath);

	std::string GetFileExtension(const std::string& filePath);
	std::string GetFileName(const std::string& filePath);
	std::string GetFileNameWithoutExtension(const std::string& filePath);
	std::string GetFileDirectory(const std::string& filePath);

	std::string LoadFile(const std::filesystem::path& path);
	std::pair<std::unique_ptr<std::byte[]>, std::size_t> LoadBinaryFile(const std::filesystem::path& path);

	// - ReadTextFile_Old - устаревшее. использовать LoadFile или LoadBinaryFile
	std::string ReadTextFile_Old(const std::string& filename);
	std::string ReadShaderCode(const std::string& filename, const std::vector<std::string>& defines = {});
} // namespace io