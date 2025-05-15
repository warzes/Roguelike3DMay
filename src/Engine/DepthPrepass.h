#pragma once

class Model;

class DepthPrepass final
{
public:
	void Create(int width, int height);
	void Destroy();

	void Start(int width, int heigh, const glm::mat4& vp);
	void DrawModel(Model* model, const glm::mat4& modelMat);

	void BindTexture(uint32_t index);

private:
	void resizeFBO(int width, int height);

	GLuint m_program{ 0 };
	int    m_uniformVPLoc{ -1 };
	int    m_uniformModelLoc{ -1 };

	GLuint m_depthpassFBO{ 0 };
	GLuint m_depthpassTextureDepth{ 0 };
	int    m_width{ 0 };
	int    m_height{ 0 };
};