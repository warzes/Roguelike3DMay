#include "stdafx.h"
#include "Mesh.h"
#include "OpenGL4Cmd.h"
//=============================================================================
Mesh::Mesh(std::span<const MeshVertex> vertices, std::span<const uint32_t> indices, PhongMaterial* material)
{
	m_vertexCount = static_cast<uint32_t>(vertices.size());
	m_indicesCount = static_cast<uint32_t>(indices.size());

	m_vertexBuffer = new gl::Buffer(vertices);
	m_indexBuffer = new gl::Buffer(indices);
	m_material = material;

	// compute aabb
	for (size_t index_id = 0; index_id < m_indicesCount; index_id++)
	{
		m_aabb.CombinePoint(vertices[indices[index_id]].position);
	}
}
//=============================================================================
Mesh::~Mesh()
{
	delete m_vertexBuffer;
	delete m_indexBuffer;
}
//=============================================================================
void Mesh::Bind()
{
	gl::Cmd::BindVertexBuffer(0, *m_vertexBuffer, 0, sizeof(MeshVertex));
	gl::Cmd::BindIndexBuffer(*m_indexBuffer, gl::IndexType::UInt);
	gl::Cmd::DrawIndexed(m_indicesCount, 1, 0, 0, 0);
}
//=============================================================================