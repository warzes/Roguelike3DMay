#include "stdafx.h"
#include "Mesh.h"
//=============================================================================
Mesh::Mesh(std::span<const MeshVertex> vertices, std::span<const uint32_t> indices, Material* material)
{
	m_vertexCount = static_cast<uint32_t>(vertices.size());
	m_indicesCount = static_cast<uint32_t>(indices.size());
}
//=============================================================================