#include "stdafx.h"
#include "PipelineShadowMapping.h"
#include "OpenGL4Wrapper.h"
//=============================================================================
PipelineShadowMapping::PipelineShadowMapping(int depthWidth, int depthHeight)
	: m_depthWidth(depthWidth)
	, m_depthHeight(depthHeight)
{
	init();
	initQuad();
}
//=============================================================================
PipelineShadowMapping::~PipelineShadowMapping()
{
	glDeleteProgram(m_depthShader);
	glDeleteProgram(m_debugShader);
	glDeleteTextures(1, &m_depthTexture);
	glDeleteFramebuffers(1, &m_depthFBO);

	gl4::Destroy(m_quadVBO);
	gl4::Destroy(m_quadVAO);
}
//=============================================================================
void PipelineShadowMapping::StartRenderDepth(float nearPlane, float farPlane, const glm::vec3& lightPosition, const glm::vec3& target)
{
	const glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
	const glm::mat4 lightView       = glm::lookAt(lightPosition, target, glm::vec3(0.0, 1.0, 0.0));
	m_lightSpaceMatrix = lightProjection * lightView;

	// Render depth
	glUseProgram(m_depthShader);
	gl4::SetUniform(m_lightSpaceMatrixLoc, m_lightSpaceMatrix);
	gl4::SetFrameBuffer(m_depthFBO, m_depthWidth, m_depthHeight, GL_DEPTH_BUFFER_BIT);
}
//=============================================================================
void PipelineShadowMapping::DebugDrawDepthMap()
{
	glUseProgram(m_debugShader);
	glBindTextureUnit(0, m_depthTexture);
	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
//=============================================================================
void PipelineShadowMapping::init()
{
	// shader shadow mapping depth
	{
		const char* shaderCodeVertex = R"(
#version 330 core

layout(location = 0) in vec3 aPos;

uniform mat4 LightSpaceMatrix;
uniform mat4 ModelMatrix;

void main()
{
	gl_Position = LightSpaceMatrix * ModelMatrix * vec4(aPos, 1.0);
}
)";

		const char* shaderCodeFragment = R"(
#version 330 core

void main()
{
}
)";

		m_depthShader         = gl4::CreateShaderProgram(shaderCodeVertex, shaderCodeFragment);
		m_lightSpaceMatrixLoc = gl4::GetUniformLocation(m_depthShader, "LightSpaceMatrix");
		m_modelMatrixLoc      = gl4::GetUniformLocation(m_depthShader, "ModelMatrix");
	}

	// shader shadow mapping debug
	{
		const char* shaderCodeVertex = R"(
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
	gl_Position = vec4(aPos, 1.0);
	TexCoords = aTexCoords;
}
)";

		const char* shaderCodeFragment = R"(
#version 330 core

in vec2 TexCoords;

uniform sampler2D DepthMap;

out vec4 FragColor;

void main()
{
	float depthValue = texture(DepthMap, TexCoords).r;
	FragColor = vec4(vec3(depthValue), 1.0);
}
)";

		m_debugShader = gl4::CreateShaderProgram(shaderCodeVertex, shaderCodeFragment);
		m_shadowMapLoc = gl4::GetUniformLocation(m_debugShader, "DepthMap");
		glUseProgram(m_debugShader);
		gl4::SetUniform(m_shadowMapLoc, 0);
		glUseProgram(0);
	}

	m_depthTexture = gl4::CreateDepthBuffer2D(m_depthWidth, m_depthHeight);
	m_depthFBO = gl4::CreateFrameBuffer2D(0, m_depthTexture);
}
//=============================================================================
void PipelineShadowMapping::initQuad()
{
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec2 uv;
	};

	std::vector<gl4::VertexAttribute> attribs = {
		{0, 3, GL_FLOAT, false, offsetof(Vertex, pos)},
		{1, 2, GL_FLOAT, false, offsetof(Vertex, uv)},
	};

	Vertex quadVertices[]{
		// positions		// texture Coords
		{{-1.0f,  1.0f, 0.0f}, {0.0f, 1.0f}},
		{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
		{{ 1.0f,  1.0f, 0.0f}, {1.0f, 1.0f}},
		{{ 1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
	};

	m_quadVBO = gl4::CreateBufferStorage(0, sizeof(quadVertices), quadVertices);
	m_quadVAO = gl4::CreateVertexArray(m_quadVBO, sizeof(Vertex), attribs);
}
//=============================================================================