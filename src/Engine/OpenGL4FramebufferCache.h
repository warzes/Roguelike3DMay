#pragma once

#include "OpenGL4Texture.h"

namespace gl4
{
	struct RenderInfo;
}

namespace gl4::detail
{
	struct TextureProxy final
	{
		TextureCreateInfo createInfo;
		uint32_t id;

		bool operator==(const TextureProxy&) const noexcept = default;
	};

	struct RenderAttachments final
	{
		std::vector<TextureProxy> colorAttachments{};
		std::optional<TextureProxy> depthAttachment{};
		std::optional<TextureProxy> stencilAttachment{};

		bool operator==(const RenderAttachments& rhs) const;
	};

	class FramebufferCache final
	{
	public:
		FramebufferCache() = default;
		FramebufferCache(const FramebufferCache&) = delete;
		FramebufferCache& operator=(const FramebufferCache&) = delete;
		FramebufferCache(FramebufferCache&&) noexcept = default;
		FramebufferCache& operator=(FramebufferCache&&) noexcept = default;

		uint32_t CreateOrGetCachedFramebuffer(const gl4::RenderInfo& renderInfo);

		[[nodiscard]] std::size_t Size() const
		{
			return m_framebufferCacheKey.size();
		}

		void Clear();

		void RemoveTexture(const Texture& texture);

	private:
		std::vector<RenderAttachments> m_framebufferCacheKey;
		std::vector<uint32_t>          m_framebufferCacheValue;
	};
} // namespace gl4::detail