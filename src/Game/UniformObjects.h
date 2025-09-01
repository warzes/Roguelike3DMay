#pragma once

struct alignas(16) ShadowUniforms final
{
	glm::mat4 vp;
	glm::mat4 model;
};

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

struct alignas(16) MainFragmentUniforms final
{
	glm::mat4 invView;
	glm::mat4 lightProjection;

	glm::mat4 shadowMapViewProjection0;
	glm::mat4 shadowMapViewProjection1;
	glm::mat4 shadowMapViewProjection2;
	glm::mat4 shadowMapViewProjection3;
	glm::mat4 shadowMapViewProjection4;
	glm::mat4 shadowMapViewProjection5;
	glm::mat4 shadowMapViewProjection6;
	glm::mat4 shadowMapViewProjection7;
	float directionalLightShadowMapBias;
	float pointLightShadowMapBias;

	float frustumSize;

	int  MaxNumLightSources;
};

struct alignas(16) MaterialUniforms final
{
	glm::vec4 diffuse;

	int hasDiffuseTexture{ 0 };
	int hasSpecularTexture{ 0 };
	int hasEmissionTexture{ 0 };
	int hasNormalMapTexture{ 0 };
	int hasDepthMapTexture{ 0 };

	bool noLighing{ true };
};