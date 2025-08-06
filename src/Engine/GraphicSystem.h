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

	gl::BufferId m_sphereVBO;
	gl::BufferId m_sphereIBO;
	gl::VertexArrayId m_sphereVAO;
	unsigned int m_sphereIndexCount{};

	gl::BufferId m_cubeVBO;
	//gl::Buffer m_cubeIBO;
	gl::VertexArrayId m_cubeVAO;

	gl::BufferId m_quadVBO;
	gl::VertexArrayId m_quadVAO;
};