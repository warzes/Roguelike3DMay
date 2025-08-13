#include "stdafx.h"
#include "MeshOLD.h"
#include "OpenGL4Simple.h"
#include "Engine/Log.h"
//=============================================================================
MeshOLD::MeshOLD(std::vector<MeshVertexOLD>&& vertices, std::vector<unsigned int>&& indices, std::unordered_map<TextureType, TextureFile>&& textures)
	: m_vertices(std::move(vertices))
	, m_indices(std::move(indices))
	, m_textureMap(std::move(textures))
{
	setupMesh();
}
//=============================================================================
void MeshOLD::Delete()
{
	gl::Destroy(m_vao);
	gl::Destroy(m_vbo);
	gl::Destroy(m_ibo);
}
//=============================================================================
void MeshOLD::AddTextureIfEmpty(TextureType tType, const std::string& filePath)
{
	if (m_textureMap.contains(tType)) // C++20 Feature
	{
		return;
	}
	TextureFile texture;
	texture.id = gl::LoadTexture2D(filePath.c_str());
	texture.name = filePath;
	m_textureMap[tType] = texture;
}
//=============================================================================
void MeshOLD::Draw(GLuint shaderProgram, bool skipTexture) const
{
	if (!skipTexture)
	{
		glUseProgram(shaderProgram);
		// Currently only supports one texture per type
		for (unsigned int i = 0; i < TextureMapper::NUM_TEXTURE_TYPE; ++i) // Iterate over TextureType elements
		{
			TextureType tType = static_cast<TextureType>(i + 1); // Casting
			auto it = m_textureMap.find(tType);
			if (it == m_textureMap.end())
				continue;

			std::string name = TextureMapper::GetTextureString(tType) + "1";
			glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()), (int)i);
			const auto& texture = it->second;
			glBindTextureUnit(i, texture.id);
		}
	}

	// Draw mesh
	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
//=============================================================================
void MeshOLD::setupMesh()
{
	std::vector<gl::VertexAttributeRaw> attribs = {
		{0, 3, GL_FLOAT, false, offsetof(MeshVertexOLD, Position)},
		{1, 3, GL_FLOAT, false, offsetof(MeshVertexOLD, Normal)},
		{2, 2, GL_FLOAT, false, offsetof(MeshVertexOLD, TexCoords)},
		{3, 3, GL_FLOAT, false, offsetof(MeshVertexOLD, Tangent)},
		{4, 3, GL_FLOAT, false, offsetof(MeshVertexOLD, Bitangent)},
		{5, 4, GL_INT,   false, offsetof(MeshVertexOLD, BoneIDs)},
		{6, 4, GL_FLOAT, false, offsetof(MeshVertexOLD, Weights)},
	};


	m_vbo = gl::CreateBufferStorage(0, m_vertices);
	m_ibo = gl::CreateBufferStorage(0, m_indices);
	m_vao = gl::CreateVertexArray(m_vbo, m_ibo, sizeof(MeshVertexOLD), attribs);

#if _DEBUG
	Print("Mesh vertex count " + std::to_string(m_vertices.size()));
	for (auto& it : m_textureMap)
	{
		Print("Texture " + std::to_string(static_cast<int>(it.first)) + ", " + it.second.name);
	}
#endif
}
//=============================================================================