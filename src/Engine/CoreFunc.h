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

template <typename T>
[[nodiscard]] float Lerp(float a, float b, float f)
{
	return a + f * (b - a);
}

template <typename T>
[[nodiscard]] inline T RandomNumber()
{
	static std::uniform_real_distribution<T> distribution(0.0, 1.0);
	static std::random_device rd;
	static std::mt19937 generator(rd());
	return distribution(generator);
}

// Returns a random real in [min,max)
template <typename T>
[[nodiscard]] inline T RandomNumber(T min, T max)
{
	return min + (max - min) * RandomNumber<T>();
}

std::string GetFileExtension(const std::string& filePath);
std::string GetFileName(const std::string& filePath);
std::string GetFileNameWithoutExtension(const std::string& filePath);
std::string GetFileDirectory(const std::string& filePath);

std::string ReadTextFile(const std::string& filename);
std::string ReadShaderCode(const std::string& filename, const std::vector<std::string>& defines = {});

