#include "stdafx.h"
#include "DepthPrepass.h"
#include "OpenGL4Low.h"
#include "Model.h"
//=============================================================================
void DepthPrepass::Create(int width, int height)
{
	const char* shaderCodeVertex = R"(
#version 460 core

layout (location = 0) in vec3 aVertexPosition;

uniform mat4 VP;
uniform mat4 model;

void main() {
	gl_Position = VP * model * vec4(aVertexPosition, 1.0);
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

void main()
{
}
)";

	m_program = gl4::CreateShaderProgram(shaderCodeVertex, shaderCodeFragment);
	m_uniformVPLoc = gl4::GetUniformLocation(m_program, "VP");
	m_uniformModelLoc = gl4::GetUniformLocation(m_program, "model");

	// Depth buffer 
	resizeFBO(width, height);
}
//=============================================================================
void DepthPrepass::Destroy()
{
	gl4::Destroy(m_program);
	gl4::Destroy(m_depthpassFBO);
	glDeleteTextures(1, &m_depthpassTextureDepth);
	m_depthpassTextureDepth = 0;
}
//=============================================================================
void DepthPrepass::Start(int width, int height, const glm::mat4& vp)
{
	if (m_width != width || m_height != height)
	{
		resizeFBO(width, height);
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	gl4::SetFrameBuffer(m_depthpassFBO, width, height, GL_DEPTH_BUFFER_BIT);
	glUseProgram(m_program);
	gl4::SetUniform(m_uniformVPLoc, vp);
}
//=============================================================================
void DepthPrepass::DrawModel(Model* model, const glm::mat4& modelMat)
{
	gl4::SetUniform(m_uniformModelLoc, modelMat);
	model->Draw(m_program, true);
}
//=============================================================================
void DepthPrepass::BindTexture(uint32_t index)
{
	glBindTextureUnit(index, m_depthpassTextureDepth);
}
//=============================================================================
void DepthPrepass::resizeFBO(int width, int height)
{
	if (gl4::IsValid(m_depthpassFBO)) gl4::Destroy(m_depthpassFBO);
	if (m_depthpassTextureDepth) glDeleteTextures(1, &m_depthpassTextureDepth);

	gl4::TextureParameter param = {};
	param.minFilter = GL_NEAREST;
	param.magFilter = GL_NEAREST;
	param.wrap = GL_CLAMP_TO_BORDER;
	param.genMipMap = false;
	param.dataType = GL_FLOAT;
	m_depthpassTextureDepth = gl4::CreateTexture2D(GL_DEPTH_COMPONENT32F, width, height, nullptr, param);
	constexpr GLfloat border[]{ 1.0f, 1.0f, 1.0f, 1.0f };
	glTextureParameterfv(m_depthpassTextureDepth, GL_TEXTURE_BORDER_COLOR, border);

	m_depthpassFBO = gl4::CreateFrameBuffer2D(0, m_depthpassTextureDepth);

	m_width = width;
	m_height = height;
}
//=============================================================================
