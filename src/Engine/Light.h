#pragma once

class Light
{
public:
	Light(glm::vec3 position, glm::vec3 color);

	glm::vec3 Position{};
	glm::vec3 Color{};
};