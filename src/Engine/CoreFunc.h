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

///*
//id = 0;
//HashCombine(id, first_index);
//HashCombine(id, index_count);
//*/
//template <class T>
//inline void HashCombine(size_t& seed, const T& v)
//{
//	std::hash<T> hasher;
//	glm::detail::hash_combine(seed, hasher(v));
//}

/*
std::size_t seed = 0;
HashCombine(seed, h1, h2, h3);
*/
void HashCombine([[maybe_unused]] std::size_t& seed) {}
template <typename T, typename... Rest>
void HashCombine(std::size_t& seed, const T& v, Rest... rest)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	HashCombine(seed, rest...);
}