#pragma once

struct alignas(16) SceneDataBlock final
{
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	glm::aligned_vec3 cameraPosition;
};

struct alignas(16) ModelDataBlock final
{
	glm::mat4 modelMatrix;
};

struct alignas(16) LightBlock final
{
	struct DirectionalLight
	{
		glm::aligned_vec3 direction;
		glm::aligned_vec3 color;
		glm::aligned_f32  intensity;
		glm::aligned_mat4 shadowMatrix;
	} dirLight;
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
inline UniformsWrapper<LightBlock> DirectionalLightDataUBO;
inline UniformsWrapper<SMFragmentBlock> SMFragmentDataUBO;