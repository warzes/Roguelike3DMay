#include "stdafx.h"
#include "Random.h"
#include "CoreMath.h"
//=============================================================================
float Random::Hash(float a)
{
	return math::Fract(sinf(a + 1.951f) * 43758.5453123f);
}
//=============================================================================
glm::vec3 Random::Hash(const glm::vec3& a)
{
	return math::Fract(glm::sin(a + 1.951f) * 43758.5453123f);
}
//=============================================================================
float Random::Lcg(float a)
{
	uint32_t uintA = (uint32_t)a;

	if (uintA <= 1)
		return 1;

	static uint32_t M = 4294967296;
	static uint32_t A = 214013;
	static uint32_t C = 2531011;

	return (A * (uint32_t)Random::Lcg(uintA - 1) + C) % A;
}
//=============================================================================
glm::vec3 Random::Lcg(const glm::vec3&)
{
	return glm::vec3(0.0f);
}
//=============================================================================