#include "stdafx.h"
#include "ShadowPassManager.h"
//=============================================================================
bool ShadowPassManager::Init()
{
	gl4::TextureCreateInfo createInfo{
		.imageType = gl4::ImageType::Tex2D,
		.format = gl4::Format::D32_FLOAT,
		.extent = { m_shadow.width, m_shadow.height, 1},
		.mipLevels = 1,
		.arrayLayers = 1,
		.sampleCount = gl4::SampleCount::Samples1,
	};
	m_shadow.depthTexture = new gl4::Texture(createInfo, "ShadowDepth");

	m_shadow.rtAttachment = new gl4::RenderDepthStencilAttachment{
		.texture = *m_shadow.depthTexture,
		.loadOp = gl4::AttachmentLoadOp::Clear,
		.clearValue = {.depth = 1.0f},
	};

	//gl4::Viewport view{ .drawRect = {m_shadow.width, m_shadow.height} };
	m_shadow.viewport = new gl4::RenderInfo( { /*.viewport = view,*/ .depthAttachment = *m_shadow.rtAttachment } );

	return true;
}
//=============================================================================
void ShadowPassManager::Close()
{
	delete m_shadow.depthTexture;
	delete m_shadow.rtAttachment;
	delete m_shadow.viewport;
}
//=============================================================================