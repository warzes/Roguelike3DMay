#pragma once

#include "OpenGL4Simple.h"

class ModelOLD;

class DepthPrepass final
{
public:
	void Create(int width, int height);
	void Destroy();

	void Start(int width, int heigh, const glm::mat4& vp);
	void DrawModel(ModelOLD* model, const glm::mat4& modelMat);

	void BindTexture(uint32_t index);

private:
	void resizeFBO(int width, int height);

	gl::ShaderProgramId m_program{ 0 };
	int    m_uniformVPLoc{ -1 };
	int    m_uniformModelLoc{ -1 };

	gl::FrameBufferId m_depthpassFBO{ 0 };
	GLuint m_depthpassTextureDepth{ 0 };
	int    m_width{ 0 };
	int    m_height{ 0 };
};