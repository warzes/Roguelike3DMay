#include "stdafx.h"
#include "PipelineBloom.h"
#include "OpenGL4Simple.h"
#include "Engine/Log.h"
//=============================================================================
uint16_t GetWindowWidth();
uint16_t GetWindowHeight();
//=============================================================================
PipelineBloom::PipelineBloom(uint32_t /*blurIteration*/)
{
	// first pass shaders
	{
		const char* shaderCodeVertex = R"(
#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT 
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vs_out.FragPos = mat3(model) * aPos;
	vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
	vs_out.TexCoords = aTexCoords;
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

		const char* shaderCodeFragment = R"(
#version 460 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

in VS_OUT 
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_metalness1;
uniform sampler2D texture_roughness1;
uniform sampler2D texture_ao1;
uniform sampler2D texture_emissive1;

uniform vec3 lightPos;
uniform vec3 viewPos;

// Blinn-Phong with glTF model
void main()
{
	vec3 color = texture(texture_diffuse1, fs_in.TexCoords).rgb;
	vec3 emissive = texture(texture_emissive1, fs_in.TexCoords).rgb;

	// ambient
	vec3 ambient = 0.05 * color;
	
	// diffuse
	vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	vec3 normal = normalize(fs_in.Normal);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * color;
	
	// specular
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = 0.0;

	vec3 halfwayDir = normalize(lightDir + viewDir);  
	spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

	vec3 specular = vec3(0.3) * spec; // assuming bright white light color
	FragColor = vec4(ambient + diffuse + specular, 1.0);

	BrightColor = vec4(emissive, 1.0);
}
)";

		m_mainShader = gl::CreateShaderProgram(shaderCodeVertex, shaderCodeFragment);

	}

	// blur shaders
	{
		const char* shaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
	TexCoords = aTexCoords;
	gl_Position = vec4(aPos, 1.0);
}
)";

		const char* shaderCodeFragment = R"(
#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D image;

uniform bool horizontal;
uniform float weight[5] = float[](0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main()
{
	vec2 tex_offset = 1.0 / textureSize(image, 0); // gets size of single texel
	vec3 result = texture(image, TexCoords).rgb * weight[0];
	if (horizontal)
	{
		for (int i = 1; i < 5; ++i)
		{
			result += texture(image, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
			result += texture(image, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
		}
	}
	else
	{
		for (int i = 1; i < 5; ++i)
		{
			result += texture(image, TexCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
			result += texture(image, TexCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
		}
	}
	FragColor = vec4(result, 1.0);
}
)";

		m_shaderBlur = gl::CreateShaderProgram(shaderCodeVertex, shaderCodeFragment);

	}

	// final shaders
	{
		const char* shaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
	TexCoords = aTexCoords;
	gl_Position = vec4(aPos, 1.0);
}
)";

		const char* shaderCodeFragment = R"(
#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;

uniform float exposure;

void main()
{
	const float gamma = 2.2;
	vec3 hdrColor = texture(scene, TexCoords).rgb;
	vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
	hdrColor += bloomColor; // additive blending
	// Tone mapping
	//vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
	// Also gamma correct while we're at it       
	//result = pow(result, vec3(1.0 / gamma));
	FragColor = vec4(hdrColor, 1.0);
	//FragColor = vec4(bloomColor, 1.0);
}
)";

		m_shaderFinal = gl::CreateShaderProgram(shaderCodeVertex, shaderCodeFragment);

	}

	initQuad();

	// Set up Bloom pipeline
	constexpr int numMipmaps = 1;

	// Configure (floating point) framebuffers
	glCreateFramebuffers(1, &m_hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFBO);
	// Create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
	glCreateTextures(GL_TEXTURE_2D, 2, m_colorBuffers.data());
	for (uint32_t i = 0; i < 2; i++)
	{
		glTextureParameteri(m_colorBuffers[i], GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
		glTextureParameteri(m_colorBuffers[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_colorBuffers[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_colorBuffers[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_colorBuffers[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureStorage2D(m_colorBuffers[i], numMipmaps, GL_RGBA16F, GetWindowWidth(), GetWindowHeight());
		// Attach texture to framebuffer
		glNamedFramebufferTexture(m_hdrFBO, GL_COLOR_ATTACHMENT0 + i, m_colorBuffers[i], 0);
	}

	// Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	constexpr uint32_t attachments[2]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glNamedFramebufferDrawBuffers(m_hdrFBO, 2, attachments);
	if (glCheckNamedFramebufferStatus(m_hdrFBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		Error("Framebuffer not complete!");
	}

	// Create and attach depth buffer (renderbuffer)
	uint32_t rboDepth{};
	glCreateRenderbuffers(1, &rboDepth);
	glNamedRenderbufferStorage(rboDepth, GL_DEPTH_COMPONENT, GetWindowWidth(), GetWindowHeight());
	glNamedFramebufferRenderbuffer(m_hdrFBO, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Ping-pong-framebuffer for blurring
	glCreateFramebuffers(2, m_pingpongFBO.data());
	glCreateTextures(GL_TEXTURE_2D, 2, m_pingpongColorbuffers.data());
	for (uint32_t i = 0; i < 2; i++)
	{
		glTextureParameteri(m_pingpongColorbuffers[i], GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
		glTextureParameteri(m_pingpongColorbuffers[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_pingpongColorbuffers[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_pingpongColorbuffers[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTextureParameteri(m_pingpongColorbuffers[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureStorage2D(m_pingpongColorbuffers[i], numMipmaps, GL_RGBA16F, GetWindowWidth(), GetWindowHeight());

		glNamedFramebufferTexture(m_pingpongFBO[i], GL_COLOR_ATTACHMENT0, m_pingpongColorbuffers[i], 0);

		// Check if framebuffers are complete
		if (glCheckNamedFramebufferStatus(m_pingpongFBO[i], GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			Fatal("Framebuffer not complete!");
		}
	}

	// Shader
	glUseProgram(m_shaderBlur);
	gl::SetUniform(m_shaderBlur, "image", 0);
	glUseProgram(m_shaderFinal);
	gl::SetUniform(m_shaderFinal, "scene", 0);
	gl::SetUniform(m_shaderFinal, "bloomBlur", 1);
}
//=============================================================================
void PipelineBloom::StartFirstPass(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& cameraPosition, const glm::vec3& lightPosition) const
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(m_mainShader);
	gl::SetUniform(m_mainShader, "projection", projection);
	gl::SetUniform(m_mainShader, "view", view);
	gl::SetUniform(m_mainShader, "viewPos", cameraPosition);
	gl::SetUniform(m_mainShader, "lightPos", lightPosition);
}
//=============================================================================
void PipelineBloom::EndFirstPass() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
//=============================================================================
void PipelineBloom::StartBlurPass()
{
	// Blur pass
	m_horizontal = true;
	glUseProgram(m_shaderBlur);
	for (uint32_t i = 0; i < m_blurIteration; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFBO[m_horizontal]);
		gl::SetUniform(m_shaderBlur, "horizontal", m_horizontal);
		glBindTextureUnit(0,
			i == 0 ?
			m_colorBuffers[1] :
			m_pingpongColorbuffers[!m_horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
		renderQuad();
		m_horizontal = !m_horizontal;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
//=============================================================================
void PipelineBloom::RenderComposite() const
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(m_shaderFinal);
	glBindTextureUnit(0, m_colorBuffers[0]);
	glBindTextureUnit(1, m_pingpongColorbuffers[!m_horizontal]);
	renderQuad();
}
//=============================================================================
void PipelineBloom::initQuad()
{
	// Quad
	constexpr float quadVertices[]{
		// Positions		// Texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};

	glCreateBuffers(1, &m_quadVBO);
	glNamedBufferStorage(m_quadVBO, sizeof(quadVertices), &quadVertices, GL_DYNAMIC_STORAGE_BIT);
	glCreateVertexArrays(1, &m_quadVAO);
	glVertexArrayVertexBuffer(m_quadVAO, 0, m_quadVBO, 0, 5 * sizeof(float));

	glEnableVertexArrayAttrib(m_quadVAO, 0);
	glEnableVertexArrayAttrib(m_quadVAO, 1);

	glVertexArrayAttribFormat(m_quadVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(m_quadVAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float));

	glVertexArrayAttribBinding(m_quadVAO, 0, 0);
	glVertexArrayAttribBinding(m_quadVAO, 1, 0);
}
//=============================================================================
void PipelineBloom::renderQuad() const
{
	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
//=============================================================================