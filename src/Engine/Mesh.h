#pragma once

#include "MeshVertex.h"
#include "Material.h"
#include "AABB.h"
#include "OpenGL4Buffer.h"

struct MeshCreateInfo final
{
	std::vector<MeshVertex> vertices;
	std::vector<uint32_t> indices;
	PhongMaterial* material{ nullptr };
};

// TODO: сделать возможность хранить буфер вершин/индексов в Model, а здесь хранить смещения в буфере
class Mesh final
{
public:
	Mesh() = default;
	Mesh(const MeshCreateInfo& createInfo);
	Mesh(std::span<const MeshVertex> vertices,
		std::span<const uint32_t> indices,
		PhongMaterial* material);
	~Mesh();

	uint32_t GetVertexCount() const { return m_vertexCount; }
	uint32_t GetIndexCount() const { return m_indicesCount; }
	const AABB& GetAABB() const { return m_aabb; }

	void Bind();

	PhongMaterial* GetMaterial() { return m_material; }

private:
	uint32_t       m_vertexCount{ 0 };
	uint32_t       m_indicesCount{ 0 };

	gl::Buffer* m_vertexBuffer{ nullptr };
	gl::Buffer* m_indexBuffer{ nullptr };
	PhongMaterial* m_material{ nullptr };
	AABB        m_aabb{};
};