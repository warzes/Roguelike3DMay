#pragma once

#include "OpenGL4Sampler.h"

namespace gl::detail
{
	class SamplerCache final
	{
	public:
		SamplerCache() = default;
		SamplerCache(const SamplerCache&) = delete;
		SamplerCache& operator=(const SamplerCache&) = delete;
		SamplerCache(SamplerCache&&) noexcept = default;
		SamplerCache& operator=(SamplerCache&&) noexcept = default;

		[[nodiscard]] gl::Sampler CreateOrGetCachedTextureSampler(const gl::SamplerState& samplerState);
		[[nodiscard]] size_t Size() const { return m_samplerCache.size(); }
		void Clear();

	private:
		std::unordered_map<gl::SamplerState, gl::Sampler> m_samplerCache;
	};

} // namespace gl::detail