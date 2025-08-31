#include "stdafx.h"
#include "Mesh.h"
#include "OpenGL4Cmd.h"
//=============================================================================
Mesh::Mesh(const MeshCreateInfo& ci)
	: Mesh(ci.vertices, ci.indices, ci.material)
{
}
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
void Mesh::Bind(std::optional<gl::Sampler> sampler)
{
	if (m_material && sampler.has_value())
	{
		if (m_material->diffuseTexture)
			gl::Cmd::BindSampledImage(0, *m_material->diffuseTexture, *sampler);
		if (m_material->specularTexture)
			gl::Cmd::BindSampledImage(1, *m_material->specularTexture, *sampler);
		if (m_material->emissionTexture)
			gl::Cmd::BindSampledImage(2, *m_material->emissionTexture, *sampler);
		if (m_material->normalTexture)
			gl::Cmd::BindSampledImage(3, *m_material->normalTexture, *sampler);
		if (m_material->depthTexture)
			gl::Cmd::BindSampledImage(4, *m_material->depthTexture, *sampler);
	}

	gl::Cmd::BindVertexBuffer(0, *m_vertexBuffer, 0, sizeof(MeshVertex));
	gl::Cmd::BindIndexBuffer(*m_indexBuffer, gl::IndexType::UInt);
	gl::Cmd::DrawIndexed(m_indicesCount, 1, 0, 0, 0);
}
//=============================================================================