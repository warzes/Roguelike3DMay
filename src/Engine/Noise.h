#pragma once

inline float FastInvSqrt(float a)
{
	float half = a * 0.5f;
	int i = *(int*)&a;
	i = 0x5f3759df - (i >> 1);
	a = *(float*)&i;
	a = a * (1.5f - half * a * a);
	return a;
}

inline glm::vec3 FastInvSqrt(glm::vec3 a)
{
	glm::vec3 half = a * 0.5f;

	int i = *(int*)&a.x;
	i = 0x5f3759df - (i >> 1);
	a.x = *(float*)&i;
	a.x = a.x * (1.5f - half.x * a.x * a.x);

	i = *(int*)&a.y;
	i = 0x5f3759df - (i >> 1);
	a.y = *(float*)&i;
	a.y = a.y * (1.5f - half.y * a.y * a.y);

	i = *(int*)&a.z;
	i = 0x5f3759df - (i >> 1);
	a.z = *(float*)&i;
	a.z = a.z * (1.5f - half.z * a.z * a.z);

	return a;
}

inline glm::vec4 FastInvSqrt(glm::vec4 a)
{
	glm::vec4 half = a * 0.5f;

	int i = *(int*)&a.x;
	i = 0x5f3759df - (i >> 1);
	a.x = *(float*)&i;
	a.x = a.x * (1.5f - half.x * a.x * a.x);

	i = *(int*)&a.y;
	i = 0x5f3759df - (i >> 1);
	a.y = *(float*)&i;
	a.y = a.y * (1.5f - half.y * a.y * a.y);

	i = *(int*)&a.z;
	i = 0x5f3759df - (i >> 1);
	a.z = *(float*)&i;
	a.z = a.z * (1.5f - half.z * a.z * a.z);

	i = *(int*)&a.w;
	i = 0x5f3759df - (i >> 1);
	a.w = *(float*)&i;
	a.w = a.w * (1.5f - half.w * a.w * a.w);

	return a;
}

class Noise final
{
public:
	static float PerlinNoise(glm::vec3 pos, float freq);
	static float PerlinFbm(const glm::vec3& pos, float freq, int octaveCount);

	static float WorleyNoise(const glm::vec3& pos, float cellCount);
	static float WorleyFbm(const glm::vec3& pos, float cellCount, float freqs[3]);

	static float CurlNoise(const glm::vec3& pos);

private:
	static glm::vec4 permute(const glm::vec4& a);
	static glm::vec3 fade(const glm::vec3& a);
	static glm::vec4 fade(const glm::vec4& a);
};