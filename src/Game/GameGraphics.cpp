#include "stdafx.h"
#include "GameGraphics.h"
#include "GameApp.h"
#include "GameSceneManager.h"
//=============================================================================
bool GameGraphics::Init(GameApp* gameApp)
{
	m_gameApp = gameApp;
	return true;
}
//=============================================================================
void GameGraphics::Close()
{
	m_colorBuffer = {};
	m_depthBuffer = {};
}
//=============================================================================
void GameGraphics::Update(float deltaTime)
{
}
//=============================================================================
void GameGraphics::Render(GameSceneManager& scene)
{
	auto depthAttachment2 = gl4::RenderDepthStencilAttachment{
		  .texture = m_depthBuffer2.value(),
		  .loadOp = gl4::AttachmentLoadOp::Clear,
		  .clearValue = {.depth = 1.0f},
	};

	gl4::BeginRendering({ .depthAttachment = depthAttachment2 });
	{
		scene.DrawInDepth();
	}
	gl4::EndRendering();

	auto colorAttachment = gl4::RenderColorAttachment{
		.texture    = m_colorBuffer.value(),
		.loadOp     = gl4::AttachmentLoadOp::Clear,
		.clearValue = {.1f, .5f, .8f, 1.0f},
	};
	auto depthAttachment = gl4::RenderDepthStencilAttachment{
	  .texture    = m_depthBuffer.value(),
	  .loadOp     = gl4::AttachmentLoadOp::Clear,
	  .clearValue = {.depth = 1.0f},
	};

	gl4::BeginRendering( { .colorAttachments = {&colorAttachment, 1}, .depthAttachment = depthAttachment });
	{
		scene.Draw();
	}
	gl4::EndRendering();

	gl4::BlitTextureToSwapchain(*m_colorBuffer, {}, {}, m_colorBuffer->Extent(), { GetWindowWidth(), GetWindowHeight(), 1 }, gl4::MagFilter::Nearest);
}
//=============================================================================
void GameGraphics::Resize(uint16_t width, uint16_t height)
{
	m_colorBuffer = gl4::CreateTexture2D({ width, height }, gl4::Format::R8G8B8A8_SRGB, "ColorBuffer");
	m_depthBuffer = gl4::CreateTexture2D({ width, height }, gl4::Format::D32_FLOAT, "DepthBuffer");
	m_depthBuffer2 = gl4::CreateTexture2D({ width, height }, gl4::Format::D32_FLOAT, "DepthBuffer2");
}
//=============================================================================