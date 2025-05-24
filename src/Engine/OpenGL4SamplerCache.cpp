#include "stdafx.h"
#include "OpenGL4SamplerCache.h"
#include "OpenGL4ApiToEnum.h"
#include "Hash.h"
#include "Log.h"
//=============================================================================
std::size_t std::hash<gl4::SamplerState>::operator()(const gl4::SamplerState& k) const noexcept
{
	auto rtup = std::make_tuple(
		k.minFilter,
		k.magFilter,
		k.addressModeU,
		k.addressModeV,
		k.addressModeW,
		k.borderColor,
		k.anisotropy,
		k.compareEnable,
		k.compareOp,
		k.lodBias,
		k.minLod,
		k.maxLod);
	return detail::hashing::hash<decltype(rtup)>{}(rtup);
}
//=============================================================================
gl4::Sampler gl4::detail::SamplerCache::CreateOrGetCachedTextureSampler(const SamplerState& samplerState)
{
	if (auto it = m_samplerCache.find(samplerState); it != m_samplerCache.end())
	{
		return it->second;
	}

	GLuint sampler{};
	glCreateSamplers(1, &sampler);

	glSamplerParameteri(sampler, GL_TEXTURE_COMPARE_MODE,
		samplerState.compareEnable ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE);

	glSamplerParameteri(sampler, GL_TEXTURE_COMPARE_FUNC, static_cast<GLint>(detail::EnumToGL(samplerState.compareOp)));

	GLint magFilter = static_cast<GLint>(detail::EnumToGL(samplerState.magFilter));
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, magFilter);

	GLint minFilter = static_cast<GLint>(detail::EnumToGL(samplerState.minFilter));
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, minFilter);

	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, detail::EnumToGL(samplerState.addressModeU));
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, detail::EnumToGL(samplerState.addressModeV));
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, detail::EnumToGL(samplerState.addressModeW));

	// TODO: determine whether int white values should be 1 or 255
	switch (samplerState.borderColor)
	{
		case BorderColor::FloatTransparentBlack:
	{
		constexpr GLfloat color[4]{ 0, 0, 0, 0 };
		glSamplerParameterfv(sampler, GL_TEXTURE_BORDER_COLOR, color);
		break;
	}
	case BorderColor::IntTransparentBlack:
	{
		constexpr GLint color[4]{ 0, 0, 0, 0 };
		glSamplerParameteriv(sampler, GL_TEXTURE_BORDER_COLOR, color);
		break;
	}
	case BorderColor::FloatOpaqueBlack:
	{
		constexpr GLfloat color[4]{ 0, 0, 0, 1 };
		glSamplerParameterfv(sampler, GL_TEXTURE_BORDER_COLOR, color);
		break;
	}
	case BorderColor::IntOpaqueBlack:
	{
		// constexpr GLint color[4]{ 0, 0, 0, 255 };
		constexpr GLint color[4]{ 0, 0, 0, 1 };
		glSamplerParameteriv(sampler, GL_TEXTURE_BORDER_COLOR, color);
		break;
	}
	case BorderColor::FloatOpaqueWhite:
	{
		constexpr GLfloat color[4]{ 1, 1, 1, 1 };
		glSamplerParameterfv(sampler, GL_TEXTURE_BORDER_COLOR, color);
		break;
	}
	case BorderColor::IntOpaqueWhite:
	{
		// constexpr GLint color[4]{ 255, 255, 255, 255 };
		constexpr GLint color[4]{ 1, 1, 1, 1 };
		glSamplerParameteriv(sampler, GL_TEXTURE_BORDER_COLOR, color);
		break;
	}
	default: assert(0); break;
	}

	glSamplerParameterf(sampler,
		GL_TEXTURE_MAX_ANISOTROPY,
		static_cast<GLfloat>(detail::EnumToGL(samplerState.anisotropy)));

	glSamplerParameterf(sampler, GL_TEXTURE_LOD_BIAS, samplerState.lodBias);

	glSamplerParameterf(sampler, GL_TEXTURE_MIN_LOD, samplerState.minLod);

	glSamplerParameterf(sampler, GL_TEXTURE_MAX_LOD, samplerState.maxLod);

	Debug("Created Sampler with handle " + std::to_string(sampler));

	return m_samplerCache.insert({ samplerState, Sampler(sampler) }).first->second;
}
//=============================================================================
size_t gl4::detail::SamplerCache::Size() const
{
	return m_samplerCache.size();
}
//=============================================================================
void gl4::detail::SamplerCache::Clear()
{
	for (const auto& [_, sampler] : m_samplerCache)
	{
		Debug("Destroyed Sampler with handle " + std::to_string(sampler.m_id));
		glDeleteSamplers(1, &sampler.m_id);
	}

	m_samplerCache.clear();
}
//=============================================================================