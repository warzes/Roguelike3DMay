#pragma once

#include "OpenGL4Sampler.h"



namespace gl4::detail
{
	class SamplerCache
	{
	public:
		SamplerCache() = default;
		SamplerCache(const SamplerCache&) = delete;
		SamplerCache& operator=(const SamplerCache&) = delete;
		SamplerCache(SamplerCache&&) noexcept = default;
		SamplerCache& operator=(SamplerCache&&) noexcept = default;

		gl4::Sampler CreateOrGetCachedTextureSampler(const gl4::SamplerState& samplerState);
		[[nodiscard]] size_t Size() const;
		void Clear();

	private:
		std::unordered_map<gl4::SamplerState, gl4::Sampler> m_samplerCache;
	};
} // namespace gl4::detail