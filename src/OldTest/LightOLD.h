#pragma once

class LightOLD
{
public:
	LightOLD(glm::vec3 position, glm::vec3 color);

	glm::vec3 Position{};
	glm::vec3 Color{};
};