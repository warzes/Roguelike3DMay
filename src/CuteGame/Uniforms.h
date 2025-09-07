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

struct alignas(16) SMFragmentBlock final
{
	glm::aligned_mat4 viewMat;
	glm::aligned_vec3 albedoScaler;
	glm::aligned_vec2 depthClampPara;
	glm::aligned_vec3 emissionColor;
	glm::aligned_f32 alphaTestThreshold;
	bool depthOnly;
};

inline UniformsWrapper<SceneDataBlock> SceneDataUBO;
inline UniformsWrapper<ModelDataBlock> ModelDataUBO;
inline UniformsWrapper<SMFragmentBlock> SMFragmentDataUBO;