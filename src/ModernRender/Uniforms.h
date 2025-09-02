#pragma once

struct alignas(16) SceneDataBlock final
{
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
};

struct alignas(16) ModelDataBlock final
{
	glm::mat4 modelMatrix;
};

inline UniformsWrapper<SceneDataBlock> SceneDataUBO;
inline UniformsWrapper<ModelDataBlock> ModelDataUBO;