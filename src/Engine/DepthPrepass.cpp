#include "stdafx.h"
#include "DepthPrepass.h"
#include "OpenGL4Simple.h"
#include "ModelOLD.h"
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

	m_program = gl::CreateShaderProgram(shaderCodeVertex, shaderCodeFragment);
	m_uniformVPLoc = gl::GetUniformLocation(m_program, "VP");
	m_uniformModelLoc = gl::GetUniformLocation(m_program, "model");

	// Depth buffer 
	resizeFBO(width, height);
}
//=============================================================================
void DepthPrepass::Destroy()
{
	gl::Destroy(m_program);
	gl::Destroy(m_depthpassFBO);
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
	gl::SetFrameBuffer(m_depthpassFBO, width, height, GL_DEPTH_BUFFER_BIT);
	glUseProgram(m_program);
	gl::SetUniform(m_program, m_uniformVPLoc, vp);
}
//=============================================================================
void DepthPrepass::DrawModel(ModelOLD* model, const glm::mat4& modelMat)
{
	gl::SetUniform(m_program, m_uniformModelLoc, modelMat);
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
	if (gl::IsValid(m_depthpassFBO)) gl::Destroy(m_depthpassFBO);
	if (m_depthpassTextureDepth) glDeleteTextures(1, &m_depthpassTextureDepth);

	gl::TextureParameter param = {};
	param.minFilter = GL_NEAREST;
	param.magFilter = GL_NEAREST;
	param.wrap = GL_CLAMP_TO_BORDER;
	param.genMipMap = false;
	param.dataType = GL_FLOAT;
	m_depthpassTextureDepth = gl::CreateTexture2D(GL_DEPTH_COMPONENT32F, width, height, nullptr, param);
	constexpr GLfloat border[]{ 1.0f, 1.0f, 1.0f, 1.0f };
	glTextureParameterfv(m_depthpassTextureDepth, GL_TEXTURE_BORDER_COLOR, border);

	m_depthpassFBO = gl::CreateFrameBuffer2D(0, m_depthpassTextureDepth);

	m_width = width;
	m_height = height;
}
//=============================================================================
