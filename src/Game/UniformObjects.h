#pragma once

struct alignas(16) GlobalUniforms final
{
	glm::mat4 view;
	glm::mat4 proj;
	glm::vec3 eyePosition;
};

struct alignas(16) ObjectUniforms final
{
	glm::mat4 model;
};

struct alignas(16) MaterialUniforms final
{
	glm::vec4 diffuse;

	int hasDiffuseTexture;
	int hasSpecularTexture;
	int hasEmissionTexture;
	int hasNormalMapTexture;
	int hasDepthMapTexture;

	bool noLighing;
};