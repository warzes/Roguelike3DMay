#include "stdafx.h"
#include "OpenGL4Context.h"
#include "OpenGL4Core.h"
//=============================================================================
void gl4::ContextState::Init()
{
	properties = InitDeviceProperties();

	ResetState();
}
//=============================================================================
void gl4::ContextState::Close()
{
	vaoCache.Clear();
	samplerCache.Clear();
}
//=============================================================================
void gl4::ContextState::ResetState()
{
#error
}
//=============================================================================
void gl4::InvalidatePipelineState()
{
	assert(!gContext.isComputeActive && !gContext.isRendering);

#if defined(_DEBUG)
	ZeroResourceBindings();
#endif

	for (int i = 0; i < MAX_COLOR_ATTACHMENTS; i++)
	{
		ColorComponentFlags& flags = gContext.lastColorMask[i];
		flags = ColorComponentFlag::RGBA_BITS;
		glColorMaski(i, true, true, true, true);
	}

	gContext.lastDepthMask = false;
	glDepthMask(false);

	gContext.lastStencilMask[0] = 0;
	gContext.lastStencilMask[1] = 0;
	glStencilMask(false);

	gContext.currentFbo = 0;
	gContext.currentVao = 0;
	gContext.lastGraphicsPipeline.reset();
	gContext.initViewport = true;
	gContext.lastScissor = {};

	glEnable(GL_FRAMEBUFFER_SRGB);
	glDisable(GL_DITHER);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}
//=============================================================================
void gl4::ZeroResourceBindings()
{
	const auto& limits = gl4::gContext.properties.limits;
	for (int i = 0; i < limits.maxImageUnits; i++)
	{
		glBindImageTexture(i, 0, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);
	}

	for (int i = 0; i < limits.maxShaderStorageBufferBindings; i++)
	{
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, i, 0, 0, 0);
	}

	for (int i = 0; i < limits.maxUniformBufferBindings; i++)
	{
		glBindBufferRange(GL_UNIFORM_BUFFER, i, 0, 0, 0);
	}

	for (int i = 0; i < limits.maxCombinedTextureImageUnits; i++)
	{
		glBindTextureUnit(i, 0);
		glBindSampler(i, 0);
	}
}
//=============================================================================