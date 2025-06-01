#pragma once

#include "Material.h"

struct MeshVertex final
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

class Mesh final
{
public:
	Mesh() = default;
	Mesh(std::span<const MeshVertex> vertices,
		std::span<const uint32_t> indices,
		Material* material);

	uint32_t GetVertexCount() const { return m_vertexCount; }
	uint32_t GetIndexCount() const { return m_indicesCount; }

private:
	uint32_t m_vertexCount{ 0 };
	uint32_t m_indicesCount{ 0 };
};