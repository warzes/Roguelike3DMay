#pragma once

class GraphicSystem final
{
public:
	bool Create();
	void Destroy();

	void DrawSphere(const glm::mat4& world);

private:
	void createSphere();
	void createCube();

	GLuint m_sphereVBO;
	GLuint m_sphereIBO;
	GLuint m_sphereVAO;
	GLuint m_sphereShaderProgram;
	unsigned int m_sphereIndexCount{};

	GLuint m_cubeVBO;
	//GLuint m_cubeIBO;
	GLuint m_cubeVAO;
};