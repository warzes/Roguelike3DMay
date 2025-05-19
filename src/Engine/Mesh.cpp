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
void Mesh::Delete()
{
	gl4::Destroy(m_vao);
	gl4::Destroy(m_vbo);
	gl4::Destroy(m_ibo);
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
	std::vector<gl4::VertexAttribute> attribs = {
		{0, 3, GL_FLOAT, false, offsetof(MeshVertex, Position)},
		{1, 3, GL_FLOAT, false, offsetof(MeshVertex, Normal)},
		{2, 2, GL_FLOAT, false, offsetof(MeshVertex, TexCoords)},
		{3, 3, GL_FLOAT, false, offsetof(MeshVertex, Tangent)},
		{4, 3, GL_FLOAT, false, offsetof(MeshVertex, Bitangent)},
		{5, 4, GL_INT,   false, offsetof(MeshVertex, BoneIDs)},
		{6, 4, GL_FLOAT, false, offsetof(MeshVertex, Weights)},
	};


	m_vbo = gl4::CreateBufferStorage(0, m_vertices);
	m_ibo = gl4::CreateBufferStorage(0, m_indices);
	m_vao = gl4::CreateVertexArray(m_vbo, m_ibo, sizeof(MeshVertex), attribs);

#if _DEBUG
	Print("Mesh vertex count " + std::to_string(m_vertices.size()));
	for (auto& it : m_textureMap)
	{
		Print("Texture " + std::to_string(static_cast<int>(it.first)) + ", " + it.second.name);
	}
#endif
}
//=============================================================================