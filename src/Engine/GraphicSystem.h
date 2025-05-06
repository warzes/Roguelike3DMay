#pragma once

class GraphicSystem final
{
public:
	bool Create();
	void Destroy();

	void DrawSphere();
	void DrawCube();

private:
	void createSphere();
	void createCube();

	GLuint m_sphereVBO;
	GLuint m_sphereIBO;
	GLuint m_sphereVAO;
	unsigned int m_sphereIndexCount{};

	GLuint m_cubeVBO;
	//GLuint m_cubeIBO;
	GLuint m_cubeVAO;
};