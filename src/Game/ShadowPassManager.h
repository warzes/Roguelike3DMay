#pragma once

struct ShadowPass final
{
	gl4::Texture*                      depthTexture{ nullptr };
	gl4::RenderInfo*                   viewport{ nullptr };
	gl4::RenderDepthStencilAttachment* rtAttachment{ nullptr };
	uint32_t                           width{ 1024 };
	uint32_t                           height{ 1024 };
};

class ShadowPassManager final
{
public:
	bool Init();
	void Close();

	ShadowPass& GetShadowPass() { return m_shadow; }

private:
	ShadowPass m_shadow;
};