#include "stdafx.h"
#include "OpenGL4Sampler.h"
#include "OpenGL4Context.h"
//=============================================================================
gl4::Sampler::Sampler(const SamplerState& samplerState)
	: Sampler(gContext.samplerCache.CreateOrGetCachedTextureSampler(samplerState))
{
}
//=============================================================================