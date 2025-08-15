#pragma once

template <typename T>
[[nodiscard]] bool Contains(const std::vector<T>& vec, const T& obj) noexcept
{
	return std::find(vec.begin(), vec.end(), obj) != vec.end();
}

[[nodiscard]] constexpr float Lerp(float a, float b, float f) noexcept
{
	return a + f * (b - a);
}

inline std::mt19937& GetRandomGenerator()
{
	static std::random_device rd;
	static std::mt19937 generator(rd());
	return generator;
}

template <typename T>
[[nodiscard]] inline T RandomNumber()
{
	static std::uniform_real_distribution<T> distribution(0.0, 1.0);
	return distribution(GetRandomGenerator());
}

// Returns a random real in [min,max)
template <typename T>
[[nodiscard]] inline T RandomNumber(T min, T max)
{
	assert(min < max && "min must be less than max");
	return min + (max - min) * RandomNumber<T>();
}

/*
std::size_t seed = 0;
HashCombine(seed, h1, h2, h3);
*/
inline void HashCombine([[maybe_unused]] std::size_t& seed) noexcept {}
template <typename T, typename... Rest>
inline void HashCombine(std::size_t& seed, const T& v, Rest... rest) noexcept
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	HashCombine(seed, rest...);
}