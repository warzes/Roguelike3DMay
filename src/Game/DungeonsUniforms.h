#pragma once

namespace dung
{
	struct alignas(16) SceneUniforms final
	{
		glm::mat4 projectionMatrix;
		glm::mat4 viewMatrix;
		glm::vec3 eyePosition;
		float fogStart;
		float fogEnd;
		glm::vec3 fogColor;
	};

	struct alignas(16) ModelObjUniforms final
	{
		glm::mat4 modelMatrix;
		glm::mat3 normalMatrix;
	};

	struct alignas(16) LightUniforms final
	{
		glm::mat4 pointLights[8];
		int activeLights;
	};

} // namespace dung