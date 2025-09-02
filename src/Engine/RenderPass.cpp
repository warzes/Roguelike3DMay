#include "stdafx.h"
#include "RenderPass.h"
//=============================================================================
void RenderPass::SetName(const std::string& colorBuffer, const std::string& depthBuffer)
{
	m_colorBufferName = colorBuffer;
	m_depthBufferName = depthBuffer;
}
//=============================================================================
void RenderPass::SetSize(uint16_t width, uint16_t height)
{
	m_width = width;
	m_height = height;
	m_fboColorTex = gl::CreateTexture2D({ width, height }, gl::Format::R8G8B8A8_SRGB, m_colorBufferName);
	m_fboDepthTex = gl::CreateTexture2D({ width, height }, gl::Format::D32_FLOAT, m_depthBufferName);
}
//=============================================================================
void RenderPass::Close()
{
	m_fboColorTex = {};
	m_fboDepthTex = {};
}
//=============================================================================
void RenderPass::Begin(const glm::vec3& clearColor, float clearDepth)
{
	auto sceneColorAttachment = gl::RenderColorAttachment{
		.texture = *m_fboColorTex,
		.loadOp = gl::AttachmentLoadOp::Clear,
		.clearValue = { clearColor, 1.0f },
	};

	auto sceneDepthAttachment = gl::RenderDepthStencilAttachment{
		.texture = *m_fboDepthTex,
		.loadOp = gl::AttachmentLoadOp::Clear,
		.clearValue = {.depth = clearDepth},
	};
	gl::BeginRendering({ .colorAttachments = {&sceneColorAttachment, 1}, .depthAttachment = sceneDepthAttachment });
}
//=============================================================================
void RenderPass::End()
{
	gl::EndRendering();
}
//=============================================================================