#include "stdafx.h"
#include "OpenGL4FramebufferCache.h"
#include "OpenGL4Render.h"
#include "Log.h"
//=============================================================================
inline gl::detail::RenderAttachments getAttachments(const gl::RenderInfo& renderInfo)
{
	gl::detail::RenderAttachments attachments;
	for (const auto& colorAttachment : renderInfo.colorAttachments)
	{
		attachments.colorAttachments.emplace_back(gl::detail::TextureProxy{
			colorAttachment.texture.get().GetCreateInfo(),
			colorAttachment.texture.get().Handle(),
			});
	}
	if (renderInfo.depthAttachment)
	{
		attachments.depthAttachment.emplace(gl::detail::TextureProxy{
			renderInfo.depthAttachment->texture.get().GetCreateInfo(),
			renderInfo.depthAttachment->texture.get().Handle(),
			});
	}
	if (renderInfo.stencilAttachment)
	{
		attachments.stencilAttachment.emplace(gl::detail::TextureProxy{
			renderInfo.stencilAttachment->texture.get().GetCreateInfo(),
			renderInfo.stencilAttachment->texture.get().Handle(),
			});
	}

	return attachments;
}
//=============================================================================
uint32_t gl::detail::FramebufferCache::CreateOrGetCachedFramebuffer(const gl::RenderInfo& renderInfo)
{
	RenderAttachments attachments = getAttachments(renderInfo);

	for (size_t i = 0; i < m_framebufferCacheKey.size(); i++)
	{
		if (m_framebufferCacheKey[i] == attachments)
			return m_framebufferCacheValue[i];
	}

	uint32_t fbo{};
	glCreateFramebuffers(1, &fbo);
	if (!fbo) return 0;

	std::vector<GLenum> drawBuffers;
	for (size_t i = 0; i < attachments.colorAttachments.size(); i++)
	{
		const auto& attachment = attachments.colorAttachments[i];
		glNamedFramebufferTexture(fbo, static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i), attachment.id, 0);
		drawBuffers.push_back(static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i));
	}

	if (drawBuffers.empty())
	{
		// Отключаем вывод в цветовые буферы
		// TODO: нужно ли это вообще?
		glNamedFramebufferDrawBuffer(fbo, GL_NONE);
		//glNamedFramebufferReadBuffer(fbo, GL_NONE); // TODO:
	}
	else
		glNamedFramebufferDrawBuffers(fbo, static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());

	if (attachments.depthAttachment && attachments.stencilAttachment &&
		attachments.depthAttachment == attachments.stencilAttachment)
	{
		glNamedFramebufferTexture(fbo, GL_DEPTH_STENCIL_ATTACHMENT, attachments.depthAttachment->id, 0);
	}
	else
	{
		if (attachments.depthAttachment)
		{
			glNamedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, attachments.depthAttachment->id, 0);
		}

		if (attachments.stencilAttachment)
		{
			glNamedFramebufferTexture(fbo, GL_STENCIL_ATTACHMENT, attachments.stencilAttachment->id, 0);
		}
	}

	if (glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		Fatal("Framebuffer not complete!");
		return 0;
	}

	Debug("Created Framebuffer with handle " + std::to_string(fbo));

	m_framebufferCacheKey.emplace_back(std::move(attachments));
	return m_framebufferCacheValue.emplace_back(fbo);
}
//=============================================================================
void gl::detail::FramebufferCache::Clear()
{
	for (const auto& fbo : m_framebufferCacheValue)
	{
		Debug("Destroyed Framebuffer with handle " + std::to_string(fbo));
		glDeleteFramebuffers(1, &fbo);
	}

	m_framebufferCacheKey.clear();
	m_framebufferCacheValue.clear();
}
//=============================================================================
// Must be called when a texture is deleted, otherwise the cache becomes invalid.
void gl::detail::FramebufferCache::RemoveTexture(const Texture& texture)
{
	const TextureProxy texp = { texture.GetCreateInfo(), texture.Handle() };

	for (size_t i = 0; i < m_framebufferCacheKey.size(); i++)
	{
		const auto attachments = m_framebufferCacheKey[i];

		for (const auto& ci : attachments.colorAttachments)
		{
			if (texp == ci)
			{
				m_framebufferCacheKey.erase(m_framebufferCacheKey.begin() + static_cast<const __int64>(i));
				auto fboIt = m_framebufferCacheValue.begin() + static_cast<const __int64>(i);
				glDeleteFramebuffers(1, &*fboIt);
				m_framebufferCacheValue.erase(fboIt);
				i--;
				break;
			}
		}

		if (texp == attachments.depthAttachment || texp == attachments.stencilAttachment)
		{
			m_framebufferCacheKey.erase(m_framebufferCacheKey.begin() + static_cast<const __int64>(i));
			auto fboIt = m_framebufferCacheValue.begin() + static_cast<const __int64>(i);
			glDeleteFramebuffers(1, &*fboIt);
			m_framebufferCacheValue.erase(fboIt);
			i--;
			continue;
		}
	}
}
//=============================================================================
bool gl::detail::RenderAttachments::operator==(const RenderAttachments& rhs) const
{
	if (colorAttachments.size() != rhs.colorAttachments.size())
		return false;

	// Crucially, two attachments with the same address are not necessarily the same.
	// The inverse is also true: two attachments with different addresses are not necessarily different.

	for (size_t i = 0; i < colorAttachments.size(); i++)
	{
		// Color attachments must be non-null
		if (colorAttachments[i] != rhs.colorAttachments[i])
			return false;
	}

	// Nullity of the attachments differ
	if ((depthAttachment && !rhs.depthAttachment) || (!depthAttachment && rhs.depthAttachment))
		return false;
	// Both attachments are non-null, but have different values
	if (depthAttachment && rhs.depthAttachment && (*depthAttachment != *rhs.depthAttachment))
		return false;

	if ((stencilAttachment && !rhs.stencilAttachment) || (!stencilAttachment && rhs.stencilAttachment))
		return false;
	if (stencilAttachment && rhs.stencilAttachment && (*stencilAttachment != *rhs.stencilAttachment))
		return false;

	return true;
}
//=============================================================================