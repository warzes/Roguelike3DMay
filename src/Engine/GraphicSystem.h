#pragma once

#include "OpenGL4Low.h"

class GraphicSystem final
{
public:
	bool Create();
	void Destroy();

	void DrawSphere();
	void DrawCube();
	void DrawQuad();

private:
	void createSphere();
	void createCube();
	void createQuad();

	gl4::Buffer m_sphereVBO;
	gl4::Buffer m_sphereIBO;
	gl4::VertexArray m_sphereVAO;
	unsigned int m_sphereIndexCount{};

	gl4::Buffer m_cubeVBO;
	//gl4::Buffer m_cubeIBO;
	gl4::VertexArray m_cubeVAO;

	gl4::Buffer m_quadVBO;
	gl4::VertexArray m_quadVAO;
};