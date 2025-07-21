#include "stdafx.h"
#include "Shadow.h"
#include "OpenGL4Cmd.h"
//=============================================================================
bool ShadowMap::Create()
{
	if (hasCubeMap)
	{
		return false;
	}
	else
	{
		gl4::TextureCreateInfo createInfo{
			.imageType = gl4::ImageType::Tex2D,
			.format = gl4::Format::D32_FLOAT,
			.extent = { width, height, 1},
			.mipLevels = 1,
			.arrayLayers = 1,
			.sampleCount = gl4::SampleCount::Samples1,
		};
		depthTexture = new gl4::Texture(createInfo, "ShadowDepth");
	}

	// Shaders
	lightProjection = glm::ortho(-10.f, 10.f, -10.f, 10.f, -20.0f, 20.0f);
	//lightView = glm::lookAt(glm::normalize(-shadowLightPos), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1, 0, 0));
	lightView = glm::lookAt(glm::normalize(shadowLightPos), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0, 1, 0));

	return true;
}
//=============================================================================
void ShadowMap::Destroy()
{
	delete depthTexture;
}
//=============================================================================
void ShadowMap::Bind(uint32_t index, const gl4::Sampler& sampler) const
{
	gl4::Cmd::BindSampledImage(index, *depthTexture, sampler);
}
//=============================================================================