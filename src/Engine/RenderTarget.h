#pragma once

#include "OpenGL4Texture.h"
#include "OpenGL4Render.h"

struct RTAttachment final
{
	gl::Format           format;
	std::string          name;
	gl::AttachmentLoadOp loadOp;
};

class RenderTarget final
{
public:
	void Init(uint16_t width, uint16_t height, RTAttachment colors, std::optional<RTAttachment> depth);
	void Init(uint16_t width, uint16_t height, std::span<RTAttachment> colors, std::optional<RTAttachment> depth);
	void Close();

	void SetSize(uint16_t width, uint16_t height);


	void Begin(const glm::vec3& clearColor, float clearDepth = 1.0f);
	void End();

	uint16_t GetWidth() const { return m_width; }
	uint16_t GetHeight() const { return m_height; }
	const gl::Texture* GetColor(size_t id = 0) const;
	const gl::Texture* GetDepth() const;
	Extent3D GetExtent() const { return { m_width, m_height, 1 }; }

	void BlitToSwapChain(size_t id = 0);
	void BlitToTexture(); // TODO:

private:
	struct TextureAttachment final
	{
		std::string                name;
		std::optional<gl::Texture> texture;
		gl::AttachmentLoadOp       loadOp;
	};

	std::vector<TextureAttachment> m_colorTex;
	TextureAttachment              m_depthTex;
	uint16_t                       m_width{ 0 };
	uint16_t                       m_height{ 0 };
};