#pragma once

struct ShadowPassRT final
{
	gl4::Texture*                      depthTexture{ nullptr };
	gl4::RenderInfo*                   viewport{ nullptr };
	gl4::RenderDepthStencilAttachment* rtAttachment{ nullptr };
	uint32_t                           width{ 4096 };
	uint32_t                           height{ 4096 };

	glm::mat4 lightSpaceMatrix;
	glm::vec3 lightPos;
};

class ShadowPassManager final
{
public:
	bool Init();
	void Close();

	ShadowPassRT& GetShadowPass() { return m_shadow; }

private:
	ShadowPassRT m_shadow;
};