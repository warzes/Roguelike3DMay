#include "stdafx.h"
#include "GraphicSystem.h"
#include "OpenGL4Wrapper.h"
//=============================================================================
bool GraphicSystem::Create()
{
	createSphere();
	createCube();

	return true;
}
//=============================================================================
void GraphicSystem::Destroy()
{
	glDeleteBuffers(1, &m_sphereVBO);
	glDeleteBuffers(1, &m_sphereIBO);
	glDeleteVertexArrays(1, &m_sphereVAO);
}
//=============================================================================
void GraphicSystem::DrawSphere(const glm::mat4& world)
{
	glBindVertexArray(m_sphereVAO);
	glDrawElements(GL_TRIANGLE_STRIP, m_sphereIndexCount, GL_UNSIGNED_INT, 0);
}
//=============================================================================
void GraphicSystem::createSphere()
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
	};
	std::vector<gl4::VertexAttribute> attribs = {
		{0, 3, GL_FLOAT, false, offsetof(Vertex, position)},
		{1, 2, GL_FLOAT, false, offsetof(Vertex, uv)},
		{2, 3, GL_FLOAT, false, offsetof(Vertex, normal)},
	};

	std::vector<Vertex> vertices{};
	std::vector<unsigned int> indices{};

	constexpr unsigned int X_SEGMENTS = 64;
	constexpr unsigned int Y_SEGMENTS = 64;
	constexpr float PI = glm::pi<float>();

	for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
	{
		for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
		{
			const float xSegment = static_cast<float>(x) / static_cast<float>(X_SEGMENTS);
			const float ySegment = static_cast<float>(y) / static_cast<float>(Y_SEGMENTS);
			const float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
			const float yPos = std::cos(ySegment * PI);
			const float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

			Vertex vertex;
			vertex.position = { xPos, yPos, zPos };
			vertex.uv = { xSegment, ySegment };
			vertex.normal = { xPos, yPos, zPos };
			vertices.emplace_back(vertex);
		}
	}

	bool oddRow = false;
	for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
	{
		if (!oddRow) // even rows: y == 0, y == 2; and so on
		{
			for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
			{
				indices.push_back(y * (X_SEGMENTS + 1) + x);
				indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
			}
		}
		else
		{
			for (int x = X_SEGMENTS; x >= 0; --x)
			{
				indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				indices.push_back(y * (X_SEGMENTS + 1) + x);
			}
		}
		oddRow = !oddRow;
	}
	m_sphereIndexCount = static_cast<unsigned int>(indices.size());

	m_sphereVBO = gl4::CreateBuffer(0, vertices.size() * sizeof(Vertex), vertices.data());
	m_sphereIBO = gl4::CreateBuffer(0, indices.size() * sizeof(unsigned int), indices.data());
	m_sphereVAO = gl4::CreateVertexArray(m_sphereVBO, m_sphereIBO, sizeof(Vertex), attribs);
}
//=============================================================================
void GraphicSystem::createCube()
{
	// TODO: добавить индексный буфер

	const float vertices[] = {
		// Back face
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
		// Front face
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		// Left face
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		// Right face
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
		 // Bottom face
		 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		  1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
		  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		 -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		 // Top face
		 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		  1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		  1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
		  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		 -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
	};

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 uv;
	};

	std::vector<gl4::VertexAttribute> attribs = {
		{0, 3, GL_FLOAT, false, offsetof(Vertex, pos)},
		{1, 3, GL_FLOAT, false, offsetof(Vertex, color)},
		{2, 2, GL_FLOAT, false, offsetof(Vertex, uv)},
	};

	m_cubeVBO = gl4::CreateBuffer(0, sizeof(vertices), (void*)vertices);
	m_cubeVAO = gl4::CreateVertexArray(m_cubeVBO, sizeof(Vertex), attribs);
}
//=============================================================================