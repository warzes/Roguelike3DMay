#include "stdafx.h"
#include "OpenGL4Cmd.h"
#include "OpenGL4Context.h"
#include "OpenGL4ApiToEnum.h"
//=============================================================================
namespace
{
	static void GLEnableOrDisable(GLenum state, GLboolean value)
	{
		if (value) glEnable(state);
		else glDisable(state);
	}
}
//=============================================================================
void SetViewportInternal(const gl::Viewport& viewport, const gl::Viewport& lastViewport, bool initViewport); // ==> OpenGL4Render.cpp
//=============================================================================
void gl::Cmd::BindGraphicsPipeline(const GraphicsPipeline& pipeline)
{
	assert(gContext.isRendering);
	assert(pipeline.Handle() != 0);

	auto pipelineState = detail::GetGraphicsPipelineInternal(pipeline.Handle());
	assert(pipelineState);

	//////////////////////////////////////////////////////////////// shader program
	const auto& lastGraphicsPipeline = gContext.lastGraphicsPipeline;
	if (lastGraphicsPipeline != pipelineState || gContext.lastPipelineWasCompute)
	{
		glUseProgram(static_cast<GLuint>(pipeline.Handle()));
	}

	gContext.lastPipelineWasCompute = false;

	// Early-out if this was the last pipeline bound
	if (lastGraphicsPipeline == pipelineState)
	{
		return;
	}

	if (gContext.isPipelineDebugGroupPushed)
	{
		gContext.isPipelineDebugGroupPushed = false;
		glPopDebugGroup();
	}

	if (!pipelineState->name.empty())
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION,
			0,
			static_cast<GLsizei>(pipelineState->name.size()),
			pipelineState->name.data());
		gContext.isPipelineDebugGroupPushed = true;
	}

	// Always enable this.
	// The user can create a context with a non-sRGB framebuffer or create a non-sRGB view of an sRGB texture.
	if (!lastGraphicsPipeline)
	{
		glEnable(GL_FRAMEBUFFER_SRGB);
	}

	//////////////////////////////////////////////////////////////// input assembly
	const auto& ias = pipelineState->inputAssemblyState;
	if (!lastGraphicsPipeline ||
		ias.primitiveRestartEnable != lastGraphicsPipeline->inputAssemblyState.primitiveRestartEnable)
	{
		GLEnableOrDisable(GL_PRIMITIVE_RESTART_FIXED_INDEX, ias.primitiveRestartEnable);
	}
	gContext.currentTopology = ias.topology;

	//////////////////////////////////////////////////////////////// vertex input
	if (auto nextVao = gContext.vaoCache.CreateOrGetCachedVertexArray(pipelineState->vertexInputState);
		nextVao != gContext.currentVao)
	{
		gContext.currentVao = nextVao;
		glBindVertexArray(gContext.currentVao);
	}

	//////////////////////////////////////////////////////////////// tessellation
	const auto& ts = pipelineState->tessellationState;
	if (ts.patchControlPoints > 0)
	{
		if (!lastGraphicsPipeline || ts.patchControlPoints != lastGraphicsPipeline->tessellationState.patchControlPoints)
		{
			glPatchParameteri(GL_PATCH_VERTICES, static_cast<GLint>(pipelineState->tessellationState.patchControlPoints));
		}
	}

	//////////////////////////////////////////////////////////////// rasterization
	const auto& rs = pipelineState->rasterizationState;
	if (!lastGraphicsPipeline || rs.depthClampEnable != lastGraphicsPipeline->rasterizationState.depthClampEnable)
	{
		GLEnableOrDisable(GL_DEPTH_CLAMP, rs.depthClampEnable);
	}

	if (!lastGraphicsPipeline || rs.polygonMode != lastGraphicsPipeline->rasterizationState.polygonMode)
	{
		glPolygonMode(GL_FRONT_AND_BACK, detail::EnumToGL(rs.polygonMode));
	}

	if (!lastGraphicsPipeline || rs.cullMode != lastGraphicsPipeline->rasterizationState.cullMode)
	{
		GLEnableOrDisable(GL_CULL_FACE, rs.cullMode != CullMode::None);
		if (rs.cullMode != CullMode::None)
		{
			glCullFace(detail::EnumToGL(rs.cullMode));
		}
	}

	if (!lastGraphicsPipeline || rs.frontFace != lastGraphicsPipeline->rasterizationState.frontFace)
	{
		glFrontFace(detail::EnumToGL(rs.frontFace));
	}

	if (!lastGraphicsPipeline || rs.depthBiasEnable != lastGraphicsPipeline->rasterizationState.depthBiasEnable)
	{
		GLEnableOrDisable(GL_POLYGON_OFFSET_FILL, rs.depthBiasEnable);
		GLEnableOrDisable(GL_POLYGON_OFFSET_LINE, rs.depthBiasEnable);
		GLEnableOrDisable(GL_POLYGON_OFFSET_POINT, rs.depthBiasEnable);
	}

	if (!lastGraphicsPipeline ||
		rs.depthBiasSlopeFactor != lastGraphicsPipeline->rasterizationState.depthBiasSlopeFactor ||
		rs.depthBiasConstantFactor != lastGraphicsPipeline->rasterizationState.depthBiasConstantFactor)
	{
		glPolygonOffset(rs.depthBiasSlopeFactor, rs.depthBiasConstantFactor);
	}

	if (!lastGraphicsPipeline || rs.lineWidth != lastGraphicsPipeline->rasterizationState.lineWidth)
	{
		glLineWidth(rs.lineWidth);
	}

	if (!lastGraphicsPipeline || rs.pointSize != lastGraphicsPipeline->rasterizationState.pointSize)
	{
		glPointSize(rs.pointSize);
	}

	//////////////////////////////////////////////////////////////// multisample
	const auto& ms = pipelineState->multisampleState;
	if (!lastGraphicsPipeline || ms.sampleShadingEnable != lastGraphicsPipeline->multisampleState.sampleShadingEnable)
	{
		GLEnableOrDisable(GL_SAMPLE_SHADING, ms.sampleShadingEnable);
	}

	if (!lastGraphicsPipeline || ms.minSampleShading != lastGraphicsPipeline->multisampleState.minSampleShading)
	{
		glMinSampleShading(ms.minSampleShading);
	}

	if (!lastGraphicsPipeline || ms.sampleMask != lastGraphicsPipeline->multisampleState.sampleMask)
	{
		GLEnableOrDisable(GL_SAMPLE_MASK, ms.sampleMask != 0xFFFFFFFF);
		glSampleMaski(0, ms.sampleMask);
	}

	if (!lastGraphicsPipeline || ms.alphaToCoverageEnable != lastGraphicsPipeline->multisampleState.alphaToCoverageEnable)
	{
		GLEnableOrDisable(GL_SAMPLE_ALPHA_TO_COVERAGE, ms.alphaToCoverageEnable);
	}

	if (!lastGraphicsPipeline || ms.alphaToOneEnable != lastGraphicsPipeline->multisampleState.alphaToOneEnable)
	{
		GLEnableOrDisable(GL_SAMPLE_ALPHA_TO_ONE, ms.alphaToOneEnable);
	}

	//////////////////////////////////////////////////////////////// depth + stencil
	const auto& ds = pipelineState->depthState;
	if (!lastGraphicsPipeline || ds.depthTestEnable != lastGraphicsPipeline->depthState.depthTestEnable)
	{
		GLEnableOrDisable(GL_DEPTH_TEST, ds.depthTestEnable);
	}

	if (!lastGraphicsPipeline || ds.depthWriteEnable != lastGraphicsPipeline->depthState.depthWriteEnable)
	{
		if (ds.depthWriteEnable != gContext.lastDepthMask)
		{
			glDepthMask(ds.depthWriteEnable);
			gContext.lastDepthMask = ds.depthWriteEnable;
		}
	}

	if (!lastGraphicsPipeline || ds.depthCompareOp != lastGraphicsPipeline->depthState.depthCompareOp)
	{
		glDepthFunc(detail::EnumToGL(ds.depthCompareOp));
	}

	const auto& ss = pipelineState->stencilState;
	if (!lastGraphicsPipeline || ss.stencilTestEnable != lastGraphicsPipeline->stencilState.stencilTestEnable)
	{
		GLEnableOrDisable(GL_STENCIL_TEST, ss.stencilTestEnable);
	}

	// Stencil front
	if (!lastGraphicsPipeline || !lastGraphicsPipeline->stencilState.stencilTestEnable ||
		ss.front != lastGraphicsPipeline->stencilState.front)
	{
		glStencilOpSeparate(GL_FRONT,
			detail::EnumToGL(ss.front.failOp),
			detail::EnumToGL(ss.front.depthFailOp),
			detail::EnumToGL(ss.front.passOp));
		glStencilFuncSeparate(GL_FRONT, detail::EnumToGL(ss.front.compareOp), ss.front.reference, ss.front.compareMask);
		if (gContext.lastStencilMask[0] != ss.front.writeMask)
		{
			glStencilMaskSeparate(GL_FRONT, ss.front.writeMask);
			gContext.lastStencilMask[0] = ss.front.writeMask;
		}
	}

	// Stencil back
	if (!lastGraphicsPipeline || !lastGraphicsPipeline->stencilState.stencilTestEnable ||
		ss.back != lastGraphicsPipeline->stencilState.back)
	{
		glStencilOpSeparate(GL_BACK,
			detail::EnumToGL(ss.back.failOp),
			detail::EnumToGL(ss.back.depthFailOp),
			detail::EnumToGL(ss.back.passOp));
		glStencilFuncSeparate(GL_BACK, detail::EnumToGL(ss.back.compareOp), ss.back.reference, ss.back.compareMask);
		if (gContext.lastStencilMask[1] != ss.back.writeMask)
		{
			glStencilMaskSeparate(GL_BACK, ss.back.writeMask);
			gContext.lastStencilMask[1] = ss.back.writeMask;
		}
	}

	//////////////////////////////////////////////////////////////// color blending state
	const auto& cb = pipelineState->colorBlendState;
	if (!lastGraphicsPipeline || cb.logicOpEnable != lastGraphicsPipeline->colorBlendState.logicOpEnable)
	{
		GLEnableOrDisable(GL_COLOR_LOGIC_OP, cb.logicOpEnable);
		if (!lastGraphicsPipeline || !lastGraphicsPipeline->colorBlendState.logicOpEnable ||
			(cb.logicOpEnable && cb.logicOp != lastGraphicsPipeline->colorBlendState.logicOp))
		{
			glLogicOp(detail::EnumToGL(cb.logicOp));
		}
	}

	if (!lastGraphicsPipeline || std::memcmp(cb.blendConstants,
		lastGraphicsPipeline->colorBlendState.blendConstants,
		sizeof(cb.blendConstants)) != 0)
	{
		glBlendColor(cb.blendConstants[0], cb.blendConstants[1], cb.blendConstants[2], cb.blendConstants[3]);
	}

	if (!lastGraphicsPipeline || cb.attachments.empty() != lastGraphicsPipeline->colorBlendState.attachments.empty())
	{
		GLEnableOrDisable(GL_BLEND, !cb.attachments.empty());
	}

	for (GLuint i = 0; i < static_cast<GLuint>(cb.attachments.size()); i++)
	{
		const auto& cba = cb.attachments[i];
		if (lastGraphicsPipeline && i < lastGraphicsPipeline->colorBlendState.attachments.size() &&
			cba == lastGraphicsPipeline->colorBlendState.attachments[i])
		{
			continue;
		}

		if (cba.blendEnable)
		{
			glBlendFuncSeparatei(i,
				detail::EnumToGL(cba.srcColorBlendFactor),
				detail::EnumToGL(cba.dstColorBlendFactor),
				detail::EnumToGL(cba.srcAlphaBlendFactor),
				detail::EnumToGL(cba.dstAlphaBlendFactor));
			glBlendEquationSeparatei(i, detail::EnumToGL(cba.colorBlendOp), detail::EnumToGL(cba.alphaBlendOp));
		}
		else
		{
			// "no blending" blend state
			glBlendFuncSeparatei(i, GL_SRC_COLOR, GL_ZERO, GL_SRC_ALPHA, GL_ZERO);
			glBlendEquationSeparatei(i, GL_FUNC_ADD, GL_FUNC_ADD);
		}

		if (gContext.lastColorMask[i] != cba.colorWriteMask)
		{
			glColorMaski(i,
				(cba.colorWriteMask & ColorComponentFlag::RedBit) != ColorComponentFlag::None,
				(cba.colorWriteMask & ColorComponentFlag::GreenBit) != ColorComponentFlag::None,
				(cba.colorWriteMask & ColorComponentFlag::BlueBit) != ColorComponentFlag::None,
				(cba.colorWriteMask & ColorComponentFlag::AlphaBit) != ColorComponentFlag::None);
			gContext.lastColorMask[i] = cba.colorWriteMask;
		}
	}

	gContext.lastGraphicsPipeline = pipelineState;
}
//=============================================================================
void gl::Cmd::BindComputePipeline(const ComputePipeline& pipeline)
{
	assert(gContext.isComputeActive);
	assert(pipeline.Handle() != 0);

	gContext.lastComputePipeline = detail::GetComputePipelineInternal(pipeline.Handle());
	gContext.lastPipelineWasCompute = true;

	if (gContext.isPipelineDebugGroupPushed)
	{
		gContext.isPipelineDebugGroupPushed = false;
		glPopDebugGroup();
	}

	if (!gContext.lastComputePipeline->name.empty())
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION,
			0,
			static_cast<GLsizei>(gContext.lastComputePipeline->name.size()),
			gContext.lastComputePipeline->name.data());
		gContext.isPipelineDebugGroupPushed = true;
	}

	glUseProgram(static_cast<GLuint>(pipeline.Handle()));
}
//=============================================================================
void gl::Cmd::SetViewport(const Viewport& viewport)
{
	assert(gContext.isRendering);

	SetViewportInternal(viewport, gContext.lastViewport, false);

	gContext.lastViewport = viewport;
}
//=============================================================================
void gl::Cmd::SetScissor(const Rect2D& scissor)
{
	assert(gContext.isRendering);

	if (!gContext.scissorEnabled)
	{
		glEnable(GL_SCISSOR_TEST);
		gContext.scissorEnabled = true;
	}

	if (scissor == gContext.lastScissor)
	{
		return;
	}

	glScissor(scissor.offset.x, scissor.offset.y, scissor.extent.width, scissor.extent.height);

	gContext.lastScissor = scissor;
}
//=============================================================================
void gl::Cmd::BindVertexBuffer(uint32_t bindingIndex, const Buffer& buffer, uint64_t offset, uint64_t stride)
{
	assert(gContext.isRendering);

	glVertexArrayVertexBuffer(gContext.currentVao,
		bindingIndex,
		buffer.Handle(),
		static_cast<GLintptr>(offset),
		static_cast<GLsizei>(stride));
}
//=============================================================================
void gl::Cmd::BindIndexBuffer(const Buffer& buffer, IndexType indexType)
{
	assert(gContext.isRendering);

	gContext.isIndexBufferBound = true;
	gContext.currentIndexType = indexType;
	glVertexArrayElementBuffer(gContext.currentVao, buffer.Handle());
}
//=============================================================================
void gl::Cmd::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	assert(gContext.isRendering);

	glDrawArraysInstancedBaseInstance(detail::EnumToGL(gContext.currentTopology),
		firstVertex,
		vertexCount,
		instanceCount,
		firstInstance);
}
//=============================================================================
void gl::Cmd::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	assert(gContext.isRendering);
	assert(gContext.isIndexBufferBound);

	// double cast is needed to prevent compiler from complaining about 32->64 bit pointer cast
	glDrawElementsInstancedBaseVertexBaseInstance(
		detail::EnumToGL(gContext.currentTopology),
		indexCount,
		detail::EnumToGL(gContext.currentIndexType),
		reinterpret_cast<void*>(static_cast<uintptr_t>(firstIndex * detail::GetIndexSize(gContext.currentIndexType))),
		instanceCount,
		vertexOffset,
		firstInstance);
}
//=============================================================================
void gl::Cmd::DrawIndirect(const Buffer& commandBuffer, uint64_t commandBufferOffset, uint32_t drawCount, uint32_t stride)
{
	assert(gContext.isRendering);

	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, commandBuffer.Handle());
	glMultiDrawArraysIndirect(detail::EnumToGL(gContext.currentTopology),
		reinterpret_cast<void*>(static_cast<uintptr_t>(commandBufferOffset)),
		drawCount,
		stride);
}
//=============================================================================
void gl::Cmd::DrawIndirectCount(const Buffer& commandBuffer,
	uint64_t commandBufferOffset,
	const Buffer& countBuffer,
	uint64_t countBufferOffset,
	uint32_t maxDrawCount,
	uint32_t stride)
{
	assert(gContext.isRendering);

	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, commandBuffer.Handle());
	glBindBuffer(GL_PARAMETER_BUFFER, countBuffer.Handle());
	glMultiDrawArraysIndirectCount(detail::EnumToGL(gContext.currentTopology),
		reinterpret_cast<void*>(static_cast<uintptr_t>(commandBufferOffset)),
		static_cast<GLintptr>(countBufferOffset),
		maxDrawCount,
		stride);
}
//=============================================================================
void gl::Cmd::DrawIndexedIndirect(const Buffer& commandBuffer, uint64_t commandBufferOffset, uint32_t drawCount, uint32_t stride)
{
	assert(gContext.isRendering);
	assert(gContext.isIndexBufferBound);

	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, commandBuffer.Handle());
	glMultiDrawElementsIndirect(detail::EnumToGL(gContext.currentTopology),
		detail::EnumToGL(gContext.currentIndexType),
		reinterpret_cast<void*>(static_cast<uintptr_t>(commandBufferOffset)),
		drawCount,
		stride);
}
//=============================================================================
void gl::Cmd::DrawIndexedIndirectCount(const Buffer& commandBuffer,
	uint64_t commandBufferOffset,
	const Buffer& countBuffer,
	uint64_t countBufferOffset,
	uint32_t maxDrawCount,
	uint32_t stride)
{
	assert(gContext.isRendering);
	assert(gContext.isIndexBufferBound);

	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, commandBuffer.Handle());
	glBindBuffer(GL_PARAMETER_BUFFER, countBuffer.Handle());
	glMultiDrawElementsIndirectCount(detail::EnumToGL(gContext.currentTopology),
		detail::EnumToGL(gContext.currentIndexType),
		reinterpret_cast<void*>(static_cast<uintptr_t>(commandBufferOffset)),
		static_cast<GLintptr>(countBufferOffset),
		maxDrawCount,
		stride);
}
//=============================================================================
void gl::Cmd::BindUniformBuffer(uint32_t index, const Buffer& buffer, uint64_t offset, uint64_t size)
{
	assert(gContext.isRendering || gContext.isComputeActive);

	if (size == WHOLE_BUFFER)
	{
		size = buffer.Size() - offset;
	}

	glBindBufferRange(GL_UNIFORM_BUFFER, index, buffer.Handle(), offset, size);
}
//=============================================================================
void gl::Cmd::BindUniformBuffer(std::string_view block, const Buffer& buffer, uint64_t offset, uint64_t size)
{
	const auto* uniformBlocks = gContext.isComputeActive ? &gContext.lastComputePipeline->uniformBlocks
		: &gContext.lastGraphicsPipeline->uniformBlocks;
	const auto it = std::ranges::find_if(*uniformBlocks,
		[block](const auto& pair) { return pair.first.data() == block; });

	assert(it != uniformBlocks->end());

	BindUniformBuffer(it->second, buffer, offset, size);
}
//=============================================================================
void gl::Cmd::BindStorageBuffer(uint32_t index, const Buffer& buffer, uint64_t offset, uint64_t size)
{
	assert(gContext.isRendering || gContext.isComputeActive);

	if (size == WHOLE_BUFFER)
	{
		size = buffer.Size() - offset;
	}

	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, index, buffer.Handle(), offset, size);
}
//=============================================================================
void gl::Cmd::BindStorageBuffer(std::string_view block, const Buffer& buffer, uint64_t offset, uint64_t size)
{
	const auto* storageBlocks = gContext.isComputeActive ? &gContext.lastComputePipeline->storageBlocks
		: &gContext.lastGraphicsPipeline->storageBlocks;
	const auto it = std::ranges::find_if(*storageBlocks,
		[block](const auto& pair) { return pair.first.data() == block; });

	assert(it != storageBlocks->end());

	BindStorageBuffer(it->second, buffer, offset, size);
}
//=============================================================================
void gl::Cmd::BindSampledImage(uint32_t index, const Texture& texture, const Sampler& sampler)
{
	assert(gContext.isRendering || gContext.isComputeActive);

	glBindTextureUnit(index, const_cast<Texture&>(texture).Handle());
	glBindSampler(index, sampler.Handle());
}
//=============================================================================
void gl::Cmd::BindSampledImage(std::string_view uniform, const Texture& texture, const Sampler& sampler)
{
	const auto* samplersAndImages = gContext.isComputeActive ? &gContext.lastComputePipeline->samplersAndImages
		: &gContext.lastGraphicsPipeline->samplersAndImages;
	const auto it = std::ranges::find_if(*samplersAndImages,
		[uniform](const auto& pair) { return pair.first.data() == uniform; });

	assert(it != samplersAndImages->end());

	BindSampledImage(it->second, texture, sampler);
}
//=============================================================================
void gl::Cmd::BindImage(uint32_t index, const Texture& texture, uint32_t level)
{
	assert(gContext.isRendering || gContext.isComputeActive);
	assert(level < texture.GetCreateInfo().mipLevels);
	assert(detail::IsValidImageFormat(texture.GetCreateInfo().format));

	glBindImageTexture(index,
		const_cast<Texture&>(texture).Handle(),
		level,
		GL_TRUE,
		0,
		GL_READ_WRITE,
		detail::EnumToGL(texture.GetCreateInfo().format));
}
//=============================================================================
void gl::Cmd::BindImage(std::string_view uniform, const Texture& texture, uint32_t level)
{
	const auto* samplersAndImages = gContext.isComputeActive ? &gContext.lastComputePipeline->samplersAndImages
		: &gContext.lastGraphicsPipeline->samplersAndImages;
	const auto it = std::ranges::find_if(*samplersAndImages,
		[uniform](const auto& pair) { return pair.first.data() == uniform; });

	assert(it != samplersAndImages->end());

	BindImage(it->second, texture, level);
}
//=============================================================================
void gl::Cmd::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	assert(gContext.isComputeActive);

	glDispatchCompute(groupCountX, groupCountY, groupCountZ);
}
//=============================================================================
void gl::Cmd::Dispatch(Extent3D groupCount)
{
	assert(gContext.isComputeActive);

	glDispatchCompute(groupCount.width, groupCount.height, groupCount.depth);
}
//=============================================================================
void gl::Cmd::DispatchInvocations(uint32_t invocationCountX, uint32_t invocationCountY, uint32_t invocationCountZ)
{
	DispatchInvocations(Extent3D{ invocationCountX, invocationCountY, invocationCountZ });
}
//=============================================================================
void gl::Cmd::DispatchInvocations(Extent3D invocationCount)
{
	assert(gContext.isComputeActive);

	const auto workgroupSize = gContext.lastComputePipeline->workgroupSize;
	const auto groupCount = (invocationCount + workgroupSize - 1) / workgroupSize;

	glDispatchCompute(groupCount.width, groupCount.height, groupCount.depth);
}
//=============================================================================
void gl::Cmd::DispatchInvocations(const Texture& texture, uint32_t lod)
{
	const auto imageType = texture.GetCreateInfo().imageType;
	auto extent = texture.Extent();
	extent.width >>= lod;
	extent.height >>= lod;
	if (imageType == ImageType::TexCubemap || imageType == ImageType::TexCubemapArray)
	{
		extent.depth = 6 * texture.GetCreateInfo().arrayLayers;
	}
	else if (imageType == ImageType::Tex3D)
	{
		extent.depth >>= lod;
	}
	else // texture is either an array with >= 1 layers or non-array with 1 layer.
	{
		extent.depth = texture.GetCreateInfo().arrayLayers;
	}
	DispatchInvocations(extent);
}
//=============================================================================
void gl::Cmd::DispatchIndirect(const Buffer& commandBuffer, uint64_t commandBufferOffset)
{
	assert(gContext.isComputeActive);

	glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, commandBuffer.Handle());
	glDispatchComputeIndirect(static_cast<GLintptr>(commandBufferOffset));
}
//=============================================================================