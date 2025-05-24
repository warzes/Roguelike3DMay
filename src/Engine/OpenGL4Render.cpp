#include "stdafx.h"
#include "OpenGL4Render.h"
#include "OpenGL4ApiToEnum.h"
#include "OpenGL4Texture.h"
#include "OpenGL4FramebufferCache.h"
#include "OpenGL4Context.h"
//=============================================================================
namespace
{
	// helper function
	// 	
	uint32_t getHandle(const gl4::Texture& texture)
	{
		return const_cast<gl4::Texture&>(texture).Handle();
	}

	static uint32_t MakeSingleTextureFbo(const gl4::Texture& texture, gl4::detail::FramebufferCache& fboCache)
	{
		auto format = texture.GetCreateInfo().format;

		auto depthStencil = gl4::RenderDepthStencilAttachment{ .texture = texture };
		auto color = gl4::RenderColorAttachment{ .texture = texture };
		gl4::RenderInfo renderInfo;

		if (gl4::detail::IsDepthFormat(format))
		{
			renderInfo.depthAttachment = depthStencil;
		}

		if (gl4::detail::IsStencilFormat(format))
		{
			renderInfo.stencilAttachment = depthStencil;
		}

		if (gl4::detail::IsColorFormat(format))
		{
			renderInfo.colorAttachments = { &color, 1 };
		}

		return fboCache.CreateOrGetCachedFramebuffer(renderInfo);
	}	
}
//=============================================================================
void SetViewportInternal(const gl4::Viewport& viewport, const gl4::Viewport& lastViewport, bool initViewport)
{
	if (initViewport || viewport.drawRect != lastViewport.drawRect)
	{
		glViewport(viewport.drawRect.offset.x,
			viewport.drawRect.offset.y,
			viewport.drawRect.extent.width,
			viewport.drawRect.extent.height);
	}
	if (initViewport || viewport.minDepth != lastViewport.minDepth || viewport.maxDepth != lastViewport.maxDepth)
	{
		glDepthRangef(viewport.minDepth, viewport.maxDepth);
	}
	if (initViewport || viewport.depthRange != lastViewport.depthRange)
	{
		glClipControl(GL_LOWER_LEFT, gl4::detail::EnumToGL(viewport.depthRange));
	}
}
//=============================================================================
void gl4::detail::BeginSwapchainRendering(const SwapchainRenderInfo& renderInfo)
{
	assert(!gContext.isRendering && "Cannot call BeginRendering when rendering");
	assert(!gContext.isComputeActive && "Cannot nest compute and rendering");
	gContext.isRendering = true;
	gContext.isRenderingToSwapChain = true;
	gContext.lastRenderInfo = nullptr;

#if defined(_DEBUG)
	ZeroResourceBindings();
#endif

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
		if (gContext.lastColorMask[0] != ColorComponentFlag::RGBA_BITS)
		{
			glColorMaski(0, true, true, true, true);
			gContext.lastColorMask[0] = ColorComponentFlag::RGBA_BITS;
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
	default: assert(0);
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
	default: assert(0);
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
	default: assert(0);
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
void gl4::detail::BeginRendering(const RenderInfo& renderInfo)
{
	assert(!gContext.isRendering && "Cannot call BeginRendering when rendering");
	assert(!gContext.isComputeActive && "Cannot nest compute and rendering");
	gContext.isRendering = true;

#if defined(_DEBUG)
	ZeroResourceBindings();
#endif

	gContext.lastRenderInfo = &renderInfo;

	const auto& ri = renderInfo;

	if (!ri.name.empty())
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, static_cast<GLsizei>(ri.name.size()), ri.name.data());
		gContext.isScopedDebugGroupPushed = true;
	}

	gContext.currentFbo = gContext.fboCache.CreateOrGetCachedFramebuffer(ri);
	glBindFramebuffer(GL_FRAMEBUFFER, gContext.currentFbo);

	for (GLint i = 0; i < static_cast<GLint>(ri.colorAttachments.size()); i++)
	{
		const auto& attachment = ri.colorAttachments[i];
		switch (attachment.loadOp)
		{
		case AttachmentLoadOp::Load: break;
		case AttachmentLoadOp::Clear:
		{
			if (gContext.lastColorMask[i] != ColorComponentFlag::RGBA_BITS)
			{
				glColorMaski(i, true, true, true, true);
				gContext.lastColorMask[i] = ColorComponentFlag::RGBA_BITS;
			}

			auto format = attachment.texture.get().GetCreateInfo().format;
			auto& ccv = attachment.clearValue;
			glClearNamedFramebufferfv(gContext.currentFbo, GL_COLOR, i, &ccv[0]);
		}
		case AttachmentLoadOp::DontCare:
		{
			GLenum colorAttachment = GL_COLOR_ATTACHMENT0 + i;
			glInvalidateNamedFramebufferData(gContext.currentFbo, 1, &colorAttachment);
			break;
		}
		default: assert(0);
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
		default: assert(0);
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
		default: assert(0);
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
			drawRect.extent.height =
				std::min(drawRect.extent.height, attachment.texture.get().GetCreateInfo().extent.height);
		}
		if (ri.depthAttachment)
		{
			drawRect.extent.width =
				std::min(drawRect.extent.width, ri.depthAttachment->texture.get().GetCreateInfo().extent.width);
			drawRect.extent.height =
				std::min(drawRect.extent.height, ri.depthAttachment->texture.get().GetCreateInfo().extent.height);
		}
		if (ri.stencilAttachment)
		{
			drawRect.extent.width =
				std::min(drawRect.extent.width, ri.stencilAttachment->texture.get().GetCreateInfo().extent.width);
			drawRect.extent.height =
				std::min(drawRect.extent.height, ri.stencilAttachment->texture.get().GetCreateInfo().extent.height);
		}
		viewport.drawRect = drawRect;
	}

	SetViewportInternal(viewport, gContext.lastViewport, gContext.initViewport);

	gContext.lastViewport = viewport;
	gContext.initViewport = false;
}
//=============================================================================
void gl4::detail::BeginRenderingNoAttachments(const RenderNoAttachmentsInfo& info)
{
	RenderInfo renderInfo{ .name = info.name, .viewport = info.viewport };
	BeginRendering(renderInfo);
	glNamedFramebufferParameteri(gContext.currentFbo, GL_FRAMEBUFFER_DEFAULT_WIDTH, info.framebufferSize.width);
	glNamedFramebufferParameteri(gContext.currentFbo, GL_FRAMEBUFFER_DEFAULT_HEIGHT, info.framebufferSize.height);
	glNamedFramebufferParameteri(gContext.currentFbo, GL_FRAMEBUFFER_DEFAULT_LAYERS, info.framebufferSize.depth);
	glNamedFramebufferParameteri(gContext.currentFbo, GL_FRAMEBUFFER_DEFAULT_SAMPLES, detail::EnumToGL(info.framebufferSamples));
	glNamedFramebufferParameteri(gContext.currentFbo, GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS, GL_TRUE);
}
//=============================================================================
void gl4::detail::EndRendering()
{
	assert(gContext.isRendering && "Cannot call EndRendering when not rendering");
	gContext.isRendering = false;
	gContext.isIndexBufferBound = false;
	gContext.isRenderingToSwapChain = false;

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
void gl4::detail::BeginCompute(std::string_view name)
{
	assert(!gContext.isComputeActive);
	assert(!gContext.isRendering && "Cannot nest compute and rendering");
	gContext.isComputeActive = true;

#if defined(_DEBUG)
	ZeroResourceBindings();
#endif

	if (!name.empty())
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, static_cast<GLsizei>(name.size()), name.data());
		gContext.isScopedDebugGroupPushed = true;
	}
}
//=============================================================================
void gl4::detail::EndCompute()
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
void gl4::RenderToSwapchain(const SwapchainRenderInfo& renderInfo, const std::function<void()>& func)
{
	// TODO: не нужна
	gl4::detail::BeginSwapchainRendering(renderInfo);
	func();
	gl4::detail::EndRendering();
}
//=============================================================================
void gl4::Render(const RenderInfo& renderInfo, const std::function<void()>& func)
{
	// TODO: не нужна
	gl4::detail::BeginRendering(renderInfo);
	func();
	gl4::detail::EndRendering();
}
//=============================================================================
void gl4::RenderNoAttachments(const RenderNoAttachmentsInfo& renderInfo, const std::function<void()>& func)
{
	// TODO: не нужна
	gl4::detail::BeginRenderingNoAttachments(renderInfo);
	func();
	gl4::detail::EndRendering();
}
//=============================================================================
void gl4::Compute(std::string_view name, const std::function<void()>& func)
{
	// TODO: не нужна
	gl4::detail::BeginCompute(name);
	func();
	gl4::detail::EndCompute();
}
//=============================================================================
void gl4::BlitTexture(const Texture& source,
	const Texture& target,
	Offset3D sourceOffset,
	Offset3D targetOffset,
	Extent3D sourceExtent,
	Extent3D targetExtent,
	MagFilter filter,
	AspectMask aspect)
{
	auto fboSource = MakeSingleTextureFbo(source, gContext.fboCache);
	auto fboTarget = MakeSingleTextureFbo(target, gContext.fboCache);
	glBlitNamedFramebuffer(fboSource,
		fboTarget,
		sourceOffset.x,
		sourceOffset.y,
		sourceExtent.width,
		sourceExtent.height,
		targetOffset.x,
		targetOffset.y,
		targetExtent.width,
		targetExtent.height,
		detail::AspectMaskToGL(aspect),
		detail::EnumToGL(filter));
}
//=============================================================================
void gl4::BlitTextureToSwapchain(const Texture& source,
	Offset3D sourceOffset,
	Offset3D targetOffset,
	Extent3D sourceExtent,
	Extent3D targetExtent,
	MagFilter filter,
	AspectMask aspect)
{
	auto fbo = MakeSingleTextureFbo(source, gContext.fboCache);

	glBlitNamedFramebuffer(fbo,
		0,
		sourceOffset.x,
		sourceOffset.y,
		sourceExtent.width,
		sourceExtent.height,
		targetOffset.x,
		targetOffset.y,
		targetExtent.width,
		targetExtent.height,
		detail::AspectMaskToGL(aspect),
		detail::EnumToGL(filter));
}
//=============================================================================
void gl4::CopyTexture(const CopyTextureInfo& copy)
{
	glCopyImageSubData(getHandle(copy.source),
		detail::EnumToGL(copy.source.GetCreateInfo().imageType),
		copy.sourceLevel,
		copy.sourceOffset.x,
		copy.sourceOffset.y,
		copy.sourceOffset.z,
		copy.target.Handle(),
		detail::EnumToGL(copy.target.GetCreateInfo().imageType),
		copy.targetLevel,
		copy.targetOffset.x,
		copy.targetOffset.y,
		copy.targetOffset.z,
		copy.extent.width,
		copy.extent.height,
		copy.extent.depth);
}
//=============================================================================
void gl4::MemoryBarrier(MemoryBarrierBits accessBits)
{
	glMemoryBarrier(detail::BarrierBitsToGL(accessBits));
}
//=============================================================================
void gl4::TextureBarrier()
{
	glTextureBarrier();
}
//=============================================================================
void gl4::CopyBuffer(const CopyBufferInfo& copy)
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
void gl4::CopyTextureToBuffer(const CopyTextureToBufferInfo& copy)
{
	glPixelStorei(GL_PACK_ROW_LENGTH, copy.bufferRowLength);
	glPixelStorei(GL_PACK_IMAGE_HEIGHT, copy.bufferImageHeight);

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
		copy.level,
		copy.sourceOffset.x,
		copy.sourceOffset.z,
		copy.sourceOffset.z,
		copy.extent.width,
		copy.extent.height,
		copy.extent.depth,
		format,
		type,
		static_cast<GLsizei>(copy.targetBuffer.Size()),
		reinterpret_cast<void*>(static_cast<uintptr_t>(copy.targetOffset)));
}
//=============================================================================
void gl4::CopyBufferToTexture(const CopyBufferToTextureInfo& copy)
{
	glPixelStorei(GL_UNPACK_ROW_LENGTH, copy.bufferRowLength);
	glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, copy.bufferImageHeight);

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