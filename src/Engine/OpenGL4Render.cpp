#include "stdafx.h"
#include "OpenGL4Render.h"
#include "OpenGL4ApiToEnum.h"
#include "OpenGL4Texture.h"
#include "OpenGL4FramebufferCache.h"
#include "OpenGL4Context.h"
#include "OpenGL4Buffer.h"
//=============================================================================
inline GLenum enumToGL(gl::ClipDepthRange depthRange)
{
	if (depthRange == gl::ClipDepthRange::NegativeOneToOne)
		return GL_NEGATIVE_ONE_TO_ONE;
	return GL_ZERO_TO_ONE;
}
//=============================================================================
inline uint32_t getHandle(const gl::Texture& texture)
{
	return const_cast<gl::Texture&>(texture).Handle();
}
//=============================================================================
inline bool isDepthFormat(gl::Format format)
{
	switch (format)
	{
	case gl::Format::D32_FLOAT:
	case gl::Format::D32_UNORM:
	case gl::Format::D24_UNORM:
	case gl::Format::D16_UNORM:
	case gl::Format::D32_FLOAT_S8_UINT:
	case gl::Format::D24_UNORM_S8_UINT: return true;
	default: return false;
	}
}
//=============================================================================
inline bool isStencilFormat(gl::Format format)
{
	switch (format)
	{
	case gl::Format::D32_FLOAT_S8_UINT:
	case gl::Format::D24_UNORM_S8_UINT: return true;
	default: return false;
	}
}
//=============================================================================
inline bool isColorFormat(gl::Format format)
{
	return !isDepthFormat(format) && !isStencilFormat(format);
}
//=============================================================================
inline uint32_t makeSingleTextureFbo(const gl::Texture& texture, gl::detail::FramebufferCache& fboCache)
{
	auto format       = texture.GetCreateInfo().format;
	auto depthStencil = gl::RenderDepthStencilAttachment{ .texture = texture };
	auto color        = gl::RenderColorAttachment{ .texture = texture };
	
	gl::RenderInfo renderInfo;
	if (isDepthFormat(format))   renderInfo.depthAttachment = depthStencil;
	if (isStencilFormat(format)) renderInfo.stencilAttachment = depthStencil;
	if (isColorFormat(format))   renderInfo.colorAttachments = { &color, 1 };
	return fboCache.CreateOrGetCachedFramebuffer(renderInfo);
}
//=============================================================================
void SetViewportInternal(const gl::Viewport& viewport, const gl::Viewport& lastViewport, bool initViewport)
{
	if (initViewport || viewport.drawRect != lastViewport.drawRect)
	{
		glViewport(
			static_cast<GLint>(viewport.drawRect.offset.x),
			static_cast<GLint>(viewport.drawRect.offset.y),
			static_cast<GLsizei>(viewport.drawRect.extent.width),
			static_cast<GLsizei>(viewport.drawRect.extent.height));
	}
	if (initViewport || viewport.minDepth != lastViewport.minDepth || viewport.maxDepth != lastViewport.maxDepth)
	{
		glDepthRangef(viewport.minDepth, viewport.maxDepth);
	}
	if (initViewport || viewport.depthRange != lastViewport.depthRange)
	{
		glClipControl(GL_LOWER_LEFT, enumToGL(viewport.depthRange));
	}
}
//=============================================================================
void gl::BeginSwapChainRendering(const SwapChainRenderInfo& renderInfo)
{
	assert(!gContext.isRendering && "Cannot call BeginRendering when rendering");
	assert(!gContext.isComputeActive && "Cannot nest compute and rendering");
	gContext.isRendering = true;
	gContext.lastRenderInfo = nullptr;

	const auto& ri = renderInfo;

	if (!ri.name.empty())
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, static_cast<GLsizei>(ri.name.size()), ri.name.data());
		gContext.isScopedDebugGroupPushed = true;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	switch (ri.colorLoadOp)
	{
	case AttachmentLoadOp::Load: break;
	case AttachmentLoadOp::Clear:
	{
		if (gContext.lastColorMask[0] != ColorComponentFlag::RGBABits)
		{
			glColorMaski(0, true, true, true, true);
			gContext.lastColorMask[0] = ColorComponentFlag::RGBABits;
		}
		glClearNamedFramebufferfv(0, GL_COLOR, 0, &ri.clearColorValue[0]);
		break;
	}
	case AttachmentLoadOp::DontCare:
	{
		GLenum attachment = GL_COLOR;
		glInvalidateNamedFramebufferData(0, 1, &attachment);
		break;
	}
	default: std::unreachable();
	}

	switch (ri.depthLoadOp)
	{
	case AttachmentLoadOp::Load: break;
	case AttachmentLoadOp::Clear:
	{
		if (gContext.lastDepthMask == false)
		{
			glDepthMask(true);
			gContext.lastDepthMask = true;
		}
		glClearNamedFramebufferfv(0, GL_DEPTH, 0, &ri.clearDepthValue);
		break;
	}
	case AttachmentLoadOp::DontCare:
	{
		GLenum attachment = GL_DEPTH;
		glInvalidateNamedFramebufferData(0, 1, &attachment);
		break;
	}
	default: std::unreachable();
	}

	switch (ri.stencilLoadOp)
	{
	case AttachmentLoadOp::Load: break;
	case AttachmentLoadOp::Clear:
	{
		if (gContext.lastStencilMask[0] == false || gContext.lastStencilMask[1] == false)
		{
			glStencilMask(true);
			gContext.lastStencilMask[0] = true;
			gContext.lastStencilMask[1] = true;
		}
		glClearNamedFramebufferiv(0, GL_STENCIL, 0, &ri.clearStencilValue);
		break;
	}
	case AttachmentLoadOp::DontCare:
	{
		GLenum attachment = GL_STENCIL;
		glInvalidateNamedFramebufferData(0, 1, &attachment);
		break;
	}
	default: std::unreachable();
	}

	// Framebuffer sRGB can only be disabled in this exact function
	if (!renderInfo.enableSrgb)
	{
		glDisable(GL_FRAMEBUFFER_SRGB);
		gContext.srgbWasDisabled = true;
	}

	SetViewportInternal(renderInfo.viewport, gContext.lastViewport, gContext.initViewport);

	gContext.lastViewport = renderInfo.viewport;
	gContext.initViewport = false;
}
//=============================================================================
void gl::BeginRendering(const RenderInfo& renderInfo)
{
	assert(!gContext.isRendering && "Cannot call BeginRendering when rendering");
	assert(!gContext.isComputeActive && "Cannot nest compute and rendering");
	gContext.isRendering = true;
	gContext.lastRenderInfo = &renderInfo;

//#if defined(_DEBUG)
//	ZeroResourceBindings();
//#endif
	const auto& ri = renderInfo;

	if (!ri.name.empty())
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, static_cast<GLsizei>(ri.name.size()), ri.name.data());
		gContext.isScopedDebugGroupPushed = true;
	}

	gContext.currentFbo = gContext.fboCache.CreateOrGetCachedFramebuffer(ri);
	glBindFramebuffer(GL_FRAMEBUFFER, gContext.currentFbo);

	for (size_t i = 0; i < ri.colorAttachments.size(); i++)
	{
		const auto& attachment = ri.colorAttachments[i];
		switch (attachment.loadOp)
		{
		case AttachmentLoadOp::Load: break;
		case AttachmentLoadOp::Clear:
		{
			if (gContext.lastColorMask[i] != ColorComponentFlag::RGBABits)
			{
				glColorMaski(static_cast<GLuint>(i), true, true, true, true);
				gContext.lastColorMask[i] = ColorComponentFlag::RGBABits;
			}

			auto& ccv = attachment.clearValue;
			glClearNamedFramebufferfv(gContext.currentFbo, GL_COLOR, static_cast<GLint>(i), &ccv[0]);
			break;
		}
		case AttachmentLoadOp::DontCare:
		{
			GLenum colorAttachment = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i);
			glInvalidateNamedFramebufferData(gContext.currentFbo, 1, &colorAttachment);
			break;
		}
		default: std::unreachable();
		}
	}

	if (ri.depthAttachment)
	{
		switch (ri.depthAttachment->loadOp)
		{
		case AttachmentLoadOp::Load: break;
		case AttachmentLoadOp::Clear:
		{
			// clear just depth
			if (gContext.lastDepthMask == false)
			{
				glDepthMask(true);
				gContext.lastDepthMask = true;
			}

			glClearNamedFramebufferfv(gContext.currentFbo, GL_DEPTH, 0, &ri.depthAttachment->clearValue.depth);
			break;
		}
		case AttachmentLoadOp::DontCare:
		{
			GLenum attachment = GL_DEPTH_ATTACHMENT;
			glInvalidateNamedFramebufferData(gContext.currentFbo, 1, &attachment);
			break;
		}
		default: std::unreachable();
		}
	}

	if (ri.stencilAttachment)
	{
		switch (ri.stencilAttachment->loadOp)
		{
		case AttachmentLoadOp::Load: break;
		case AttachmentLoadOp::Clear:
		{
			// clear just stencil
			if (gContext.lastStencilMask[0] == false || gContext.lastStencilMask[1] == false)
			{
				glStencilMask(true);
				gContext.lastStencilMask[0] = true;
				gContext.lastStencilMask[1] = true;
			}

			glClearNamedFramebufferiv(gContext.currentFbo, GL_STENCIL, 0, &ri.stencilAttachment->clearValue.stencil);
			break;
		}
		case AttachmentLoadOp::DontCare:
		{
			GLenum attachment = GL_STENCIL_ATTACHMENT;
			glInvalidateNamedFramebufferData(gContext.currentFbo, 1, &attachment);
			break;
		}
		default: std::unreachable();
		}
	}

	Viewport viewport{};
	if (ri.viewport)
	{
		viewport = *ri.viewport;
	}
	else
	{
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		// determine intersection of all render targets
		Rect2D drawRect{
		.offset = {},
		.extent = {std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max()},
		};
		for (const auto& attachment : ri.colorAttachments)
		{
			drawRect.extent.width = std::min(drawRect.extent.width, attachment.texture.get().GetCreateInfo().extent.width);
			drawRect.extent.height = std::min(drawRect.extent.height, attachment.texture.get().GetCreateInfo().extent.height);
		}
		if (ri.depthAttachment)
		{
			drawRect.extent.width = std::min(drawRect.extent.width, ri.depthAttachment->texture.get().GetCreateInfo().extent.width);
			drawRect.extent.height = std::min(drawRect.extent.height, ri.depthAttachment->texture.get().GetCreateInfo().extent.height);
		}
		if (ri.stencilAttachment)
		{
			drawRect.extent.width = std::min(drawRect.extent.width, ri.stencilAttachment->texture.get().GetCreateInfo().extent.width);
			drawRect.extent.height = std::min(drawRect.extent.height, ri.stencilAttachment->texture.get().GetCreateInfo().extent.height);
		}
		viewport.drawRect = drawRect;
	}

	SetViewportInternal(viewport, gContext.lastViewport, gContext.initViewport);

	gContext.lastViewport = viewport;
	gContext.initViewport = false;
}
//=============================================================================
void gl::BeginRenderingNoAttachments(const RenderNoAttachmentsInfo& info)
{
	RenderInfo renderInfo{ .name = info.name, .viewport = info.viewport };
	BeginRendering(renderInfo);
	glNamedFramebufferParameteri(gContext.currentFbo, GL_FRAMEBUFFER_DEFAULT_WIDTH, static_cast<GLint>(info.framebufferSize.width));
	glNamedFramebufferParameteri(gContext.currentFbo, GL_FRAMEBUFFER_DEFAULT_HEIGHT, static_cast<GLint>(info.framebufferSize.height));
	glNamedFramebufferParameteri(gContext.currentFbo, GL_FRAMEBUFFER_DEFAULT_LAYERS, static_cast<GLint>(info.framebufferSize.depth));
	glNamedFramebufferParameteri(gContext.currentFbo, GL_FRAMEBUFFER_DEFAULT_SAMPLES, detail::EnumToGL(info.framebufferSamples));
	glNamedFramebufferParameteri(gContext.currentFbo, GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS, GL_TRUE);
}
//=============================================================================
void gl::EndRendering()
{
	assert(gContext.isRendering && "Cannot call EndRendering when not rendering");
	gContext.isRendering = false;
	gContext.isIndexBufferBound = false;

	if (gContext.isScopedDebugGroupPushed)
	{
		gContext.isScopedDebugGroupPushed = false;
		glPopDebugGroup();
	}

	if (gContext.isPipelineDebugGroupPushed)
	{
		gContext.isPipelineDebugGroupPushed = false;
		glPopDebugGroup();
	}

	if (gContext.scissorEnabled)
	{
		glDisable(GL_SCISSOR_TEST);
		gContext.scissorEnabled = false;
	}

	if (gContext.srgbWasDisabled)
	{
		glEnable(GL_FRAMEBUFFER_SRGB);
	}
}
//=============================================================================
void gl::BeginCompute(std::string_view name)
{
	assert(!gContext.isComputeActive);
	assert(!gContext.isRendering && "Cannot nest compute and rendering");
	gContext.isComputeActive = true;

//#if defined(_DEBUG)
//	ZeroResourceBindings();
//#endif

	if (!name.empty())
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, static_cast<GLsizei>(name.size()), name.data());
		gContext.isScopedDebugGroupPushed = true;
	}
}
//=============================================================================
void gl::EndCompute()
{
	assert(gContext.isComputeActive);
	gContext.isComputeActive = false;

	if (gContext.isScopedDebugGroupPushed)
	{
		gContext.isScopedDebugGroupPushed = false;
		glPopDebugGroup();
	}

	if (gContext.isPipelineDebugGroupPushed)
	{
		gContext.isPipelineDebugGroupPushed = false;
		glPopDebugGroup();
	}
}
//=============================================================================
void gl::BlitTexture(const Texture& source, const Texture& target, Offset3D sourceOffset, Offset3D targetOffset, Extent3D sourceExtent, Extent3D targetExtent, MagFilter filter, AspectMask aspect)
{
	auto fboSource = makeSingleTextureFbo(source, gContext.fboCache);
	auto fboTarget = makeSingleTextureFbo(target, gContext.fboCache);
	glBlitNamedFramebuffer(fboSource,
		fboTarget,
		static_cast<GLint>(sourceOffset.x),
		static_cast<GLint>(sourceOffset.y),
		static_cast<GLint>(sourceExtent.width),
		static_cast<GLint>(sourceExtent.height),
		static_cast<GLint>(targetOffset.x),
		static_cast<GLint>(targetOffset.y),
		static_cast<GLint>(targetExtent.width),
		static_cast<GLint>(targetExtent.height),
		detail::AspectMaskToGL(aspect),
		detail::EnumToGL(filter));
}
//=============================================================================
void gl::BlitTextureToSwapChain(const Texture& source, Offset3D sourceOffset, Offset3D targetOffset, Extent3D sourceExtent, Extent3D targetExtent, MagFilter filter, AspectMask aspect)
{
	auto fbo = makeSingleTextureFbo(source, gContext.fboCache);

	glBlitNamedFramebuffer(fbo,
		0,
		static_cast<GLint>(sourceOffset.x),
		static_cast<GLint>(sourceOffset.y),
		static_cast<GLint>(sourceExtent.width),
		static_cast<GLint>(sourceExtent.height),
		static_cast<GLint>(targetOffset.x),
		static_cast<GLint>(targetOffset.y),
		static_cast<GLint>(targetExtent.width),
		static_cast<GLint>(targetExtent.height),
		detail::AspectMaskToGL(aspect),
		detail::EnumToGL(filter));
}
//=============================================================================
void gl::CopyTexture(const CopyTextureInfo& copy)
{
	glCopyImageSubData(getHandle(copy.source),
		detail::EnumToGL(copy.source.GetCreateInfo().imageType),
		static_cast<GLint>(copy.sourceLevel),
		static_cast<GLint>(copy.sourceOffset.x),
		static_cast<GLint>(copy.sourceOffset.y),
		static_cast<GLint>(copy.sourceOffset.z),
		copy.target.Handle(),
		detail::EnumToGL(copy.target.GetCreateInfo().imageType),
		static_cast<GLint>(copy.targetLevel),
		static_cast<GLint>(copy.targetOffset.x),
		static_cast<GLint>(copy.targetOffset.y),
		static_cast<GLint>(copy.targetOffset.z),
		static_cast<GLsizei>(copy.extent.width),
		static_cast<GLsizei>(copy.extent.height),
		static_cast<GLsizei>(copy.extent.depth));
}
//=============================================================================
void gl::MemoryBarrier(MemoryBarrierBits accessBits)
{
	glMemoryBarrier(detail::BarrierBitsToGL(accessBits));
}
//=============================================================================
void gl::TextureBarrier()
{
	glTextureBarrier();
}
//=============================================================================
void gl::CopyBuffer(const CopyBufferInfo& copy)
{
	auto size = copy.size;
	if (size == WHOLE_BUFFER)
	{
		size = copy.source.Size() - copy.sourceOffset;
	}

	glCopyNamedBufferSubData(copy.source.Handle(),
		copy.target.Handle(),
		static_cast<GLintptr>(copy.sourceOffset),
		static_cast<GLintptr>(copy.targetOffset),
		static_cast<GLsizeiptr>(size));
}
//=============================================================================
void gl::CopyTextureToBuffer(const CopyTextureToBufferInfo& copy)
{
	glPixelStorei(GL_PACK_ROW_LENGTH, static_cast<GLint>(copy.bufferRowLength));
	glPixelStorei(GL_PACK_IMAGE_HEIGHT, static_cast<GLint>(copy.bufferImageHeight));

	glBindBuffer(GL_PIXEL_PACK_BUFFER, copy.targetBuffer.Handle());

	GLenum format{};
	if (copy.format == UploadFormat::INFER_FORMAT)
	{
		format = detail::EnumToGL(detail::FormatToUploadFormat(copy.sourceTexture.GetCreateInfo().format));
	}
	else
	{
		format = detail::EnumToGL(copy.format);
	}

	GLenum type{};
	if (copy.type == UploadType::INFER_TYPE)
	{
		type = detail::FormatToTypeGL(copy.sourceTexture.GetCreateInfo().format);
	}
	else
	{
		type = detail::EnumToGL(copy.type);
	}

	glGetTextureSubImage(const_cast<Texture&>(copy.sourceTexture).Handle(),
		static_cast<GLint>(copy.level),
		static_cast<GLint>(copy.sourceOffset.x),
		static_cast<GLint>(copy.sourceOffset.z),
		static_cast<GLint>(copy.sourceOffset.z),
		static_cast<GLsizei>(copy.extent.width),
		static_cast<GLsizei>(copy.extent.height),
		static_cast<GLsizei>(copy.extent.depth),
		format,
		type,
		static_cast<GLsizei>(copy.targetBuffer.Size()),
		reinterpret_cast<void*>(static_cast<uintptr_t>(copy.targetOffset)));
}
//=============================================================================
void gl::CopyBufferToTexture(const CopyBufferToTextureInfo& copy)
{
	glPixelStorei(GL_UNPACK_ROW_LENGTH, static_cast<GLint>(copy.bufferRowLength));
	glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, static_cast<GLint>(copy.bufferImageHeight));

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, copy.sourceBuffer.Handle());

	copy.targetTexture.subImageInternal({ copy.level,
		copy.targetOffset,
		copy.extent,
		copy.format,
		copy.type,
		reinterpret_cast<void*>(static_cast<uintptr_t>(copy.sourceOffset)),
		copy.bufferRowLength,
		copy.bufferImageHeight });
}
//=============================================================================
void gl::SetUniform(unsigned id, const std::string& label, bool value)
{
	assert(id != 0);
	glProgramUniform1i(id, glGetUniformLocation(id, label.c_str()), static_cast<int>(value));
}
//=============================================================================
void gl::SetUniform(unsigned id, const std::string& label, int value)
{
	assert(id != 0);
	glProgramUniform1i(id, glGetUniformLocation(id, label.c_str()), value);
}
//=============================================================================
void gl::SetUniform(unsigned id, const std::string& label, unsigned value)
{
	assert(id != 0);
	glProgramUniform1ui(id, glGetUniformLocation(id, label.c_str()), value);
}
//=============================================================================
void gl::SetUniform(unsigned id, const std::string& label, float value)
{
	assert(id != 0);
	glProgramUniform1f(id, glGetUniformLocation(id, label.c_str()), value);
}
//=============================================================================
void gl::SetUniform(unsigned id, const std::string& label, std::span<const float> value)
{
	assert(id != 0);
	glProgramUniform1fv(id, glGetUniformLocation(id, label.c_str()), static_cast<GLsizei>(value.size()), value.data());
}
//=============================================================================
void gl::SetUniform(unsigned id, const std::string& label, const float* value, int count)
{
	assert(id != 0);
	glProgramUniform1fv(id, glGetUniformLocation(id, label.c_str()), count, value);
}
//=============================================================================
void gl::SetUniform(unsigned id, const std::string& label, std::span<const glm::vec2> value)
{
	assert(id != 0);
	glProgramUniform2fv(id, glGetUniformLocation(id, label.c_str()), static_cast<GLsizei>(value.size()), glm::value_ptr(value.front()));
}
//=============================================================================
void gl::SetUniform(unsigned id, const std::string& label, std::span<const glm::vec3> value)
{
	assert(id != 0);
	glProgramUniform3fv(id, glGetUniformLocation(id, label.c_str()), static_cast<GLsizei>(value.size()), glm::value_ptr(value.front()));
}
//=============================================================================
void gl::SetUniform(unsigned id, const std::string& label, std::span<const glm::vec4> value)
{
	assert(id != 0);
	glProgramUniform4fv(id, glGetUniformLocation(id, label.c_str()), static_cast<GLsizei>(value.size()), glm::value_ptr(value.front()));
}
//=============================================================================
void gl::SetUniform(unsigned id, const std::string& label, std::span<const int> value)
{
	assert(id != 0);
	glProgramUniform1iv(id, glGetUniformLocation(id, label.c_str()), static_cast<GLsizei>(value.size()), value.data());
}
//=============================================================================
void gl::SetUniform(unsigned id, const std::string& label, const glm::vec2& value)
{
	assert(id != 0);
	glProgramUniform2fv(id, glGetUniformLocation(id, label.c_str()), 1, glm::value_ptr(value));
}
//=============================================================================
void gl::SetUniform(unsigned id, const std::string& label, float x, float y)
{
	assert(id != 0);
	glProgramUniform2f(id, glGetUniformLocation(id, label.c_str()), x, y);
}
//=============================================================================
void gl::SetUniform(unsigned id, const std::string& label, const glm::ivec2& value)
{
	assert(id != 0);
	glProgramUniform2iv(id, glGetUniformLocation(id, label.c_str()), 1, glm::value_ptr(value));
}
//=============================================================================
void gl::SetUniform(unsigned id, const std::string& label, int x, int y)
{
	assert(id != 0);
	glProgramUniform2i(id, glGetUniformLocation(id, label.c_str()), x, y);
}
//=============================================================================
void gl::SetUniform(unsigned id, const std::string& label, const glm::vec3& value)
{
	assert(id != 0);
	glProgramUniform3fv(id, glGetUniformLocation(id, label.c_str()), 1, glm::value_ptr(value));
}
//=============================================================================
void gl::SetUniform(unsigned id, const std::string& label, float x, float y, float z)
{
	assert(id != 0);
	glProgramUniform3f(id, glGetUniformLocation(id, label.c_str()), x, y, z);
}
//=============================================================================
void gl::SetUniform(unsigned id, const std::string& label, const glm::vec4& value)
{
	assert(id != 0);
	glProgramUniform4fv(id, glGetUniformLocation(id, label.c_str()), 1, glm::value_ptr(value));
}
//=============================================================================
void gl::SetUniform(unsigned id, const std::string& label, float x, float y, float z, float w)
{
	assert(id != 0);
	glProgramUniform4f(id, glGetUniformLocation(id, label.c_str()), x, y, z, w);
}
//=============================================================================
void gl::SetUniform(unsigned id, const std::string& label, const glm::mat3& mat)
{
	assert(id != 0);
	glProgramUniformMatrix3fv(id, glGetUniformLocation(id, label.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl::SetUniform(unsigned id, const std::string& label, const glm::mat4& mat)
{
	assert(id != 0);
	glProgramUniformMatrix4fv(id, glGetUniformLocation(id, label.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================