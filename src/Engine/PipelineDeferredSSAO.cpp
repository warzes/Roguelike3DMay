#include "stdafx.h"
#include "PipelineDeferredSSAO.h"
#include "OpenGL4Wrapper.h"
#include "PipelineDeferredSSAOShaders.h"
#include "CoreFunc.h"
//=============================================================================
uint16_t GetWindowWidth();
uint16_t GetWindowHeight();
//=============================================================================
PipelineDeferredSSAO::PipelineDeferredSSAO(int kernelSize, int noiseSize)
{
	m_shaderGeometry = gl4::CreateShaderProgram(geometryVertexSource, geometryFragmentSource);

	m_shaderLighting = gl4::CreateShaderProgram(ssaoVertexSource, lightingFragmentSource);
	m_shaderSSAO = gl4::CreateShaderProgram(ssaoVertexSource, ssaoFragmentSource);
	m_shaderBlur = gl4::CreateShaderProgram(ssaoVertexSource, blurFragmentSource);

	// G Buffer
	glCreateFramebuffers(1, &m_gBufferFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_gBufferFBO); // Needed because we are not fully DSA

	constexpr uint32_t numMipmaps = 1;

	// Position
	glCreateTextures(GL_TEXTURE_2D, 1, &m_gPositionTexture);
	glTextureParameteri(m_gPositionTexture, GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
	glTextureParameteri(m_gPositionTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_gPositionTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(m_gPositionTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_gPositionTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureStorage2D(m_gPositionTexture, numMipmaps, GL_RGBA16F, GetWindowWidth(), GetWindowHeight());
	glNamedFramebufferTexture(m_gBufferFBO, GL_COLOR_ATTACHMENT0, m_gPositionTexture, 0);

	// Normal
	glCreateTextures(GL_TEXTURE_2D, 1, &m_gNormalTexture);
	glTextureParameteri(m_gNormalTexture, GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
	glTextureParameteri(m_gNormalTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_gNormalTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(m_gNormalTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_gNormalTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureStorage2D(m_gNormalTexture, numMipmaps, GL_RGBA16F, GetWindowWidth(), GetWindowHeight());
	glNamedFramebufferTexture(m_gBufferFBO, GL_COLOR_ATTACHMENT1, m_gNormalTexture, 0);

	// Color + Specular
	glCreateTextures(GL_TEXTURE_2D, 1, &m_gAlbedoTexture);
	glTextureParameteri(m_gAlbedoTexture, GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
	glTextureParameteri(m_gAlbedoTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_gAlbedoTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(m_gAlbedoTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_gAlbedoTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureStorage2D(m_gAlbedoTexture, numMipmaps, GL_RGBA8, GetWindowWidth(), GetWindowHeight());
	glNamedFramebufferTexture(m_gBufferFBO, GL_COLOR_ATTACHMENT2, m_gAlbedoTexture, 0);

	// Attachments
	constexpr uint32_t attachments[3]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glNamedFramebufferDrawBuffers(m_gBufferFBO, 3, attachments);

	// Depth render buffer
	glGenRenderbuffers(1, &m_depthRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_depthRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, GetWindowWidth(), GetWindowHeight());
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRBO);

	// Check frame buffer
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("Framebuffer not complete!\n");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// SSAO Frame buffer
	glGenFramebuffers(1, &m_ssaoFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_ssaoFBO);

	// SSAO color buffer
	glGenTextures(1, &m_ssaoColorTexture);
	glBindTexture(GL_TEXTURE_2D, m_ssaoColorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, GetWindowWidth(), GetWindowHeight(), 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ssaoColorTexture, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("SSAO Framebuffer not complete!\n");
	}

	// Blur frame buffer
	glGenFramebuffers(1, &m_ssaoBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_ssaoBlurFBO);
	glGenTextures(1, &m_ssaoBlurTexture);
	glBindTexture(GL_TEXTURE_2D, m_ssaoBlurTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, GetWindowWidth(), GetWindowHeight(), 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ssaoBlurTexture, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("SSAO Blur Framebuffer not complete!\n");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Sample kernel
	const float kernelSizeF = static_cast<float>(kernelSize);
	for (int i = 0; i < kernelSize; ++i)
	{
		glm::vec3 sample(RandomNumber<float>(-1.0f, 1.0f),
			RandomNumber<float>(-1.0f, 1.0f),
			RandomNumber<float>()); // Half hemisphere
		sample = glm::normalize(sample);
		sample *= RandomNumber<float>();
		float scale = static_cast<float>(i) / kernelSizeF;

		// Scale samples s.t. they're more aligned to center of kernel
		scale = Lerp<float>(0.1f, 1.0f, scale * scale);
		sample *= scale;
		m_ssaoKernel.push_back(sample);
	}

	// Noise texture
	const int noiseSizeSq = noiseSize * noiseSize;
	std::vector<glm::vec3> ssaoNoise(noiseSizeSq);
	for (int i = 0; i < noiseSizeSq; ++i)
	{
		glm::vec3 noise(
			RandomNumber<float>() * 2.0 - 1.0,
			RandomNumber<float>() * 2.0 - 1.0,
			0.0f);
		ssaoNoise[i] = noise;
	}

	glCreateTextures(GL_TEXTURE_2D, 1, &m_noiseTexture);
	glTextureParameteri(m_noiseTexture, GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
	glTextureParameteri(m_noiseTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_noiseTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(m_noiseTexture, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(m_noiseTexture, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureStorage2D(m_noiseTexture, numMipmaps, GL_RGB32F, noiseSize, noiseSize);
	glTextureSubImage2D(m_noiseTexture, 0, 0, 0, noiseSize, noiseSize, GL_RGB, GL_FLOAT, &ssaoNoise[0]);

	// Shader configuration
	glUseProgram(m_shaderLighting);
	gl4::SetUniform(m_shaderLighting, "gPosition", 0);
	gl4::SetUniform(m_shaderLighting, "gNormal", 1);
	gl4::SetUniform(m_shaderLighting, "gAlbedo", 2);
	gl4::SetUniform(m_shaderLighting, "ssao", 3);

	glUseProgram(m_shaderSSAO);
	gl4::SetUniform(m_shaderSSAO, "gPosition", 0);
	gl4::SetUniform(m_shaderSSAO, "gNormal", 1);
	gl4::SetUniform(m_shaderSSAO, "texNoise", 2);
	gl4::SetUniform(m_shaderSSAO, "screen_width", static_cast<float>(GetWindowWidth()));
	gl4::SetUniform(m_shaderSSAO, "screen_height", static_cast<float>(GetWindowHeight()));
	gl4::SetUniform(m_shaderSSAO, "noise_size", static_cast<float>(noiseSize));

	glUseProgram(m_shaderBlur);
	gl4::SetUniform(m_shaderBlur, "ssaoInput", 0);

	// Plane
	constexpr float quadVertices[]{
		// Positions		// Texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};
	unsigned int quadVBO = 0;
	glGenVertexArrays(1, &m_quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(m_quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}
//=============================================================================
PipelineDeferredSSAO::~PipelineDeferredSSAO()
{
	// TODO: очистка
}
//=============================================================================
void PipelineDeferredSSAO::StartGeometryPass(const glm::mat4& projection, const glm::mat4& view)
{
	// 1 Geometry pass: render scene's geometry/color data into G buffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_gBufferFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	const glm::mat4 model = glm::mat4(1.0f);
	glUseProgram(m_shaderGeometry);
	gl4::SetUniform(m_shaderGeometry, "projection", projection);
	gl4::SetUniform(m_shaderGeometry, "view", view);
	gl4::SetUniform(m_shaderGeometry, "model", model);
	gl4::SetUniform(m_shaderGeometry, "invertedNormals", 1.0f);
}
//=============================================================================
void PipelineDeferredSSAO::EndGeometryPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
//=============================================================================
void PipelineDeferredSSAO::StartSSAOPass(const glm::mat4& projection, int kernelSize, float radius, float bias)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_ssaoFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(m_shaderSSAO);
	gl4::SetUniform(m_shaderSSAO, "kernelSize", kernelSize);
	gl4::SetUniform(m_shaderSSAO, "radius", radius);
	gl4::SetUniform(m_shaderSSAO, "bias", bias);
	// Send kernel + rotation 
	for (int i = 0; i < kernelSize; ++i)
	{
		gl4::SetUniform(m_shaderSSAO, "samples[" + std::to_string(i) + "]", m_ssaoKernel[i]);
	}
	gl4::SetUniform(m_shaderSSAO, "projection", projection);
	glBindTextureUnit(0, m_gPositionTexture);
	glBindTextureUnit(1, m_gNormalTexture);
	glBindTextureUnit(2, m_noiseTexture);
	renderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
//=============================================================================
void PipelineDeferredSSAO::StartBlurPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_ssaoBlurFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(m_shaderBlur);
	glBindTextureUnit(0, m_ssaoColorTexture);
	renderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
//=============================================================================
void PipelineDeferredSSAO::StartLightingPass(const std::vector<Light>& lights, const glm::mat4& cameraView, const glm::vec3& cameraPosition)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(m_shaderLighting);
	for (unsigned int i = 0; i < lights.size(); ++i)
	{
		glm::vec3 lightPosView = glm::vec3(cameraView * glm::vec4(lights[i].Position, 1.0));
		gl4::SetUniform(m_shaderLighting, "lights[" + std::to_string(i) + "].Position", lightPosView);
		gl4::SetUniform(m_shaderLighting, "lights[" + std::to_string(i) + "].Color", lights[i].Color);
	}

	// Attenuation parameters
	constexpr float linear = 2.9f;
	constexpr float quadratic = 3.8f;
	gl4::SetUniform(m_shaderLighting, "linear", linear);
	gl4::SetUniform(m_shaderLighting, "quadratic", quadratic);
	gl4::SetUniform(m_shaderLighting, "viewPos", cameraPosition);
	glBindTextureUnit(0, m_gPositionTexture);
	glBindTextureUnit(1, m_gNormalTexture);
	glBindTextureUnit(2, m_gAlbedoTexture);
	glBindTextureUnit(3, m_ssaoBlurTexture);
	renderQuad();
}
//=============================================================================
void PipelineDeferredSSAO::Blit()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_gBufferFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, GetWindowWidth(), GetWindowHeight(), 0, 0, GetWindowWidth(), GetWindowHeight(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
//=============================================================================
void PipelineDeferredSSAO::renderQuad()
{
	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
//=============================================================================