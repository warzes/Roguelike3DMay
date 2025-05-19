#pragma once

#include "OpenGL4Wrapper.h"

class PipelineShadowMapping final
{
public:
	PipelineShadowMapping(int depthWidth, int depthHeight);
	~PipelineShadowMapping();

	void StartRenderDepth(float nearPlane, float farPlane, const glm::vec3& lightPosition, const glm::vec3& target);

	void DebugDrawDepthMap();

	void BindDepthTexture(unsigned int index) const { glBindTextureUnit(index, m_depthTexture); }

	[[nodiscard]] auto GetDepthShader() const { return m_depthShader; }
	[[nodiscard]] int              GetDepthShaderModelMatrixLoc() const { return m_modelMatrixLoc; }
	[[nodiscard]] const glm::mat4& GetLightSpaceMatrix() const { return m_lightSpaceMatrix; }

private:
	void init();
	void initQuad();

	int m_depthWidth{};
	int m_depthHeight{};

	gl4::ShaderProgram m_depthShader{ 0 };
	int m_lightSpaceMatrixLoc{ -1 };
	int m_modelMatrixLoc{ -1 };

	gl4::ShaderProgram m_debugShader{ 0 };
	int m_shadowMapLoc{ -1 };

	GLuint m_depthTexture{ 0 };
	gl4::FrameBuffer m_depthFBO{ 0 };

	gl4::Buffer m_quadVBO{ 0 };
	gl4::VertexArray m_quadVAO{ 0 };

	glm::mat4 m_lightSpaceMatrix{};
};