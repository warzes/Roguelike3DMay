#pragma once

#include "OpenGL4Texture.h"
#include "OpenGL4Render.h"

class RenderPass final
{
public:
	void SetName(const std::string& colorBuffer, const std::string& depthBuffer);
	void SetSize(uint16_t width, uint16_t height);
	void Close();

	void Begin(const glm::vec3& clearColor, float clearDepth = 1.0f);
	void End();

	const gl::Texture& GetColor() const { assert(m_fboColorTex.has_value()); return *m_fboColorTex; }
	const gl::Texture& GetDepth() const { assert(m_fboDepthTex.has_value()); return *m_fboDepthTex; }
	Extent3D GetExtent() const { return { m_width, m_height, 1 }; }
private:
	std::optional<gl::Texture> m_fboColorTex;
	std::optional<gl::Texture> m_fboDepthTex;
	std::string                m_colorBufferName;
	std::string                m_depthBufferName;
	uint16_t                   m_width{ 0 };
	uint16_t                   m_height{ 0 };
};