#pragma once

#include "LightOLD.h"
#include "OpenGL4Simple.h"

class PipelineDeferredSSAO final
{
public:
	PipelineDeferredSSAO(int kernelSize, int noiseSize);
	~PipelineDeferredSSAO();

	void StartGeometryPass(const glm::mat4& projection, const glm::mat4& view);
	void EndGeometryPass();
	void StartSSAOPass(const glm::mat4& projection, int kernelSize, float radius, float bias );
	void StartBlurPass();
	void StartLightingPass(const std::vector<LightOLD>& lights, const glm::mat4& cameraView, const glm::vec3& cameraPosition);
	void Blit();

	[[nodiscard]] auto GetGeometryShader() const
	{
		return m_shaderGeometry;
	}

private:
	void renderQuad();

	std::vector<glm::vec3> m_ssaoKernel{};

	gl::ShaderProgramId m_shaderGeometry{ 0 };
	gl::ShaderProgramId m_shaderLighting{ 0 };
	gl::ShaderProgramId m_shaderSSAO{ 0 };
	gl::ShaderProgramId m_shaderBlur{ 0 };

	GLuint m_quadVAO{ 0 };

	GLuint m_gBufferFBO{ 0 };
	GLuint m_gPositionTexture{ 0 };
	GLuint m_gNormalTexture{ 0 };
	GLuint m_gAlbedoTexture{ 0 };
	GLuint m_depthRBO{ 0 };

	GLuint m_ssaoFBO{};
	GLuint m_ssaoColorTexture{ 0 };
	GLuint m_ssaoBlurTexture{ 0 };
	GLuint m_ssaoBlurFBO{ 0 };

	GLuint m_noiseTexture{ 0 };
};