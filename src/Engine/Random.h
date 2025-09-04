#pragma once

class Random final
{
public:
	static float Hash(float a);
	static glm::vec3 Hash(const glm::vec3& a);
	static float Lcg(float a);
	static glm::vec3 Lcg(const glm::vec3& a);
};