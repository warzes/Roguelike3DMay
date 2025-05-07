#pragma once

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

	GLuint m_sphereVBO;
	GLuint m_sphereIBO;
	GLuint m_sphereVAO;
	unsigned int m_sphereIndexCount{};

	GLuint m_cubeVBO;
	//GLuint m_cubeIBO;
	GLuint m_cubeVAO;

	GLuint m_quadVBO;
	GLuint m_quadVAO;
};