#pragma once

#include "OpenGL4Sampler.h"



namespace gl::detail
{
	class SamplerCache
	{
	public:
		SamplerCache() = default;
		SamplerCache(const SamplerCache&) = delete;
		SamplerCache& operator=(const SamplerCache&) = delete;
		SamplerCache(SamplerCache&&) noexcept = default;
		SamplerCache& operator=(SamplerCache&&) noexcept = default;

		gl::Sampler CreateOrGetCachedTextureSampler(const gl::SamplerState& samplerState);
		[[nodiscard]] size_t Size() const;
		void Clear();

	private:
		std::unordered_map<gl::SamplerState, gl::Sampler> m_samplerCache;
	};
} // namespace gl::detail