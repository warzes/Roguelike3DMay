#pragma once

#include "OpenGL4Simple.h"

class PipelineBloom final
{
public:
	PipelineBloom(uint32_t blurIteration);

	void StartFirstPass(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& cameraPosition, const glm::vec3& lightPosition) const;
	void EndFirstPass() const;

	void StartBlurPass();

	void RenderComposite() const;

	[[nodiscard]] gl::ShaderProgramId GetMainShader() const { return m_mainShader; }

private:
	void initQuad();
	void renderQuad() const;

	gl::ShaderProgramId m_mainShader{ 0 };
	gl::ShaderProgramId m_shaderBlur{ 0 };
	gl::ShaderProgramId m_shaderFinal{ 0 };

	uint32_t m_blurIteration{ 0 };

	// First pass
	uint32_t m_hdrFBO{ 0 };
	std::array<uint32_t, 2> m_colorBuffers{ {0u} };

	// Blur pass
	bool m_horizontal{ 0 };
	std::array<uint32_t, 2> m_pingpongFBO{ {0u} };
	std::array<uint32_t, 2> m_pingpongColorbuffers{ {0u} };

	// Full screen quad
	uint32_t m_quadVAO{ 0 };
	uint32_t m_quadVBO{ 0 };
};