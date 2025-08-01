﻿#pragma once

#include "OpenGL4Texture.h"

// TODO: переделать чтобы создание прописывало свойства, а не наоборот как сейчас

struct ShadowMap final
{
	bool Create();
	void Destroy();

	void Bind(uint32_t index, const gl4::Sampler& sampler) const;

	gl4::Texture* depthTexture{ nullptr };
	uint32_t      width{ 4096 };
	uint32_t      height{ 4096 };
	glm::mat4     lightProjection;
	glm::mat4     lightView;
	glm::vec3     shadowLightPos;
	bool          hasCubeMap;
};