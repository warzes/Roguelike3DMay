#pragma once

#include "OpenGL4Simple.h"

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

	gl4::BufferId m_sphereVBO;
	gl4::BufferId m_sphereIBO;
	gl4::VertexArrayId m_sphereVAO;
	unsigned int m_sphereIndexCount{};

	gl4::BufferId m_cubeVBO;
	//gl4::Buffer m_cubeIBO;
	gl4::VertexArrayId m_cubeVAO;

	gl4::BufferId m_quadVBO;
	gl4::VertexArrayId m_quadVAO;
};