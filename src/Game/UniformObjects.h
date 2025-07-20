#pragma once

struct alignas(16) GlobalUniforms final
{
	glm::mat4 view;
	glm::mat4 proj;
};

struct alignas(16) ObjectUniforms final
{
	glm::mat4 model;
};