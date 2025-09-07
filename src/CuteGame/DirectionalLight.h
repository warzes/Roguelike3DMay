#pragma once

class DirectionalLight final
{
public:
	glm::vec3 position = glm::vec3(-20.0f, 10.0f, -3.0f);
	glm::vec3 targetView = glm::vec3(0.0f);


	glm::vec3 color = glm::vec3(1.0f, 0.6f, 0.2f);
	float intensity = 10.0f;

	glm::vec3 GetDirectional() const;
	glm::mat4 GetMatrix() const;
};

inline DirectionalLight gDirectionalLight;