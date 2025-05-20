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

	[[nodiscard]] gl4::ShaderProgramId GetMainShader() const { return m_mainShader; }

private:
	void initQuad();
	void renderQuad() const;

	gl4::ShaderProgramId m_mainShader{ 0 };
	gl4::ShaderProgramId m_shaderBlur{ 0 };
	gl4::ShaderProgramId m_shaderFinal{ 0 };

	uint32_t m_blurIteration{ 0 };

	// First pass
	uint32_t m_hdrFBO{ 0 };
	std::array<uint32_t, 2> m_colorBuffers{ 0 };

	// Blur pass
	bool m_horizontal{ 0 };
	std::array<uint32_t, 2> m_pingpongFBO{ 0 };
	std::array<uint32_t, 2> m_pingpongColorbuffers{ 0 };

	// Full screen quad
	uint32_t m_quadVAO{ 0 };
	uint32_t m_quadVBO{ 0 };
};