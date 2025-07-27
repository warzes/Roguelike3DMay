#include "stdafx.h"
#include "Mesh.h"
#include "OpenGL4Cmd.h"
//=============================================================================
Mesh::Mesh(std::span<const MeshVertex> vertices, std::span<const uint32_t> indices, PhongMaterial* material)
{
	m_vertexCount = static_cast<uint32_t>(vertices.size());
	m_indicesCount = static_cast<uint32_t>(indices.size());

	m_vertexBuffer = new gl4::Buffer(vertices);
	m_indexBuffer = new gl4::Buffer(indices);
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
	gl4::Cmd::BindVertexBuffer(0, *m_vertexBuffer, 0, sizeof(MeshVertex));
	gl4::Cmd::BindIndexBuffer(*m_indexBuffer, gl4::IndexType::UNSIGNED_INT);
	gl4::Cmd::DrawIndexed(m_indicesCount, 1, 0, 0, 0);
}
//=============================================================================