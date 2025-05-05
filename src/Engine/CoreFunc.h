#pragma once

template <typename T>
bool Contains(const std::vector<T>& vec, const T& obj)
{
	for (auto& e : vec)
	{
		if (e == obj) return true;
	}
	return false;
}

std::string GetFileExtension(const std::string& filePath);
std::string GetFileName(const std::string& filePath);
std::string GetFileNameWithoutExtension(const std::string& filePath);
std::string GetFileDirectory(const std::string& filePath);

std::string ReadTextFile(const std::string& filename);
std::string ReadShaderCode(const std::string& filename, const std::vector<std::string>& defines = {});