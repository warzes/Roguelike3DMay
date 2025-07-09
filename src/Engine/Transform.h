#pragma once

struct Transform
{
	glm::mat4 GetModelMatrix() const
	{
		glm::mat4 transform(1);
		transform = glm::translate(transform, translation);
		transform *= mat4_cast(rotation);
		transform = glm::scale(transform, scale);
		return transform;
	}

	glm::mat4 GetNormalMatrix() const
	{
		return glm::transpose(glm::inverse(glm::mat3(GetModelMatrix())));
	}

	glm::vec3 translation{ 0, 0, 0 };
	glm::quat rotation{ 1, 0, 0, 0 };
	glm::vec3 scale{ 1, 1, 1 };
};