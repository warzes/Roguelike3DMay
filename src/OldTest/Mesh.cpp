#include "stdafx.h"
#include "Mesh.h"
//=============================================================================
Mesh::Mesh(std::span<const MeshVertex> vertices, std::span<const uint32_t> indices, PhongMaterial* material)
{
	m_vertexCount = static_cast<uint32_t>(vertices.size());
	m_indicesCount = static_cast<uint32_t>(indices.size());

	m_vertexBuffer = new gl::Buffer(vertices);
	m_indexBuffer = new gl::Buffer(indices);
	m_material = material;
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