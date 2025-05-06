#include "stdafx.h"
#include "Mesh.h"
#include "OpenGL4Wrapper.h"
#include "Log.h"
//=============================================================================
Mesh::Mesh(std::vector<MeshVertex>&& vertices, std::vector<unsigned int>&& indices, std::unordered_map<TextureType, TextureFile>&& textures)
	: m_vertices(std::move(vertices))
	, m_indices(std::move(indices))
	, m_textureMap(std::move(textures))
{
	setupMesh();
}
//=============================================================================
void Mesh::Delete() const
{
	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo);
	glDeleteBuffers(1, &m_ibo);
}
//=============================================================================
void Mesh::AddTextureIfEmpty(TextureType tType, const std::string& filePath)
{
	if (m_textureMap.contains(tType)) // C++20 Feature
	{
		return;
	}
	TextureFile texture;
	texture.id = gl4::LoadTexture2D(filePath.c_str());
	texture.name = filePath;
	m_textureMap[tType] = texture;
}
//=============================================================================
void Mesh::Draw(GLuint shaderProgram, bool skipTexture) const
{
	if (!skipTexture)
	{
		// Currently only supports one texture per type
		for (unsigned int i = 0; i < TextureMapper::NUM_TEXTURE_TYPE; ++i) // Iterate over TextureType elements
		{
			TextureType tType = static_cast<TextureType>(i + 1); // Casting
			auto it = m_textureMap.find(tType);
			if (it == m_textureMap.end())
				continue;

			std::string name = TextureMapper::GetTextureString(tType) + "1";
			glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()), i);
			const auto& texture = it->second;
			glBindTextureUnit(i, texture.id);
		}
	}

	// Draw mesh
	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(m_indices.size()), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
//=============================================================================
void Mesh::setupMesh()
{
	// TODO: переделать под gl4
	glCreateBuffers(1, &m_vbo);
	glNamedBufferStorage(m_vbo, sizeof(MeshVertex) * m_vertices.size(), m_vertices.data(), GL_DYNAMIC_STORAGE_BIT);

	glCreateBuffers(1, &m_ibo);
	glNamedBufferStorage(m_ibo, sizeof(unsigned int) * m_indices.size(), m_indices.data(), GL_DYNAMIC_STORAGE_BIT);

	glCreateVertexArrays(1, &m_vao);
	glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(MeshVertex));
	glVertexArrayElementBuffer(m_vao, m_ibo);

	glEnableVertexArrayAttrib(m_vao, 0);
	glEnableVertexArrayAttrib(m_vao, 1);
	glEnableVertexArrayAttrib(m_vao, 2);
	glEnableVertexArrayAttrib(m_vao, 3);
	glEnableVertexArrayAttrib(m_vao, 4);
	glEnableVertexArrayAttrib(m_vao, 5);
	glEnableVertexArrayAttrib(m_vao, 6);

	glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(MeshVertex, Position));
	glVertexArrayAttribFormat(m_vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(MeshVertex, Normal));
	glVertexArrayAttribFormat(m_vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(MeshVertex, TexCoords));
	glVertexArrayAttribFormat(m_vao, 3, 3, GL_FLOAT, GL_FALSE, offsetof(MeshVertex, Tangent));
	glVertexArrayAttribFormat(m_vao, 4, 3, GL_FLOAT, GL_FALSE, offsetof(MeshVertex, Bitangent));
	glVertexArrayAttribIFormat(m_vao, 5, 4, GL_INT, offsetof(MeshVertex, BoneIDs));
	glVertexArrayAttribFormat(m_vao, 6, 4, GL_FLOAT, GL_FALSE, offsetof(MeshVertex, Weights));

	glVertexArrayAttribBinding(m_vao, 0, 0);
	glVertexArrayAttribBinding(m_vao, 1, 0);
	glVertexArrayAttribBinding(m_vao, 2, 0);
	glVertexArrayAttribBinding(m_vao, 3, 0);
	glVertexArrayAttribBinding(m_vao, 4, 0);
	glVertexArrayAttribBinding(m_vao, 5, 0);
	glVertexArrayAttribBinding(m_vao, 6, 0);

#if _DEBUG
	Print("Mesh vertex count " + std::to_string(m_vertices.size()));
	for (auto& it : m_textureMap)
	{
		Print("Texture " + std::to_string(static_cast<int>(it.first)) + ", " + it.second.name);
	}
#endif
}
//=============================================================================