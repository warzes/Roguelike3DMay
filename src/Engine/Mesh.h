#pragma once

#include "Material.h"
#include "OpenGL4Low.h"

#define MAX_BONE_INFLUENCE 4

struct MeshVertex final
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;
	int BoneIDs[MAX_BONE_INFLUENCE];   // Bone indexes which will influence this vertex
	float Weights[MAX_BONE_INFLUENCE]; // Weights from each bone
};

// TODO:
// SubMesh - геометрия меша
// SubMeshGPU - вао/вбо/ибо

class Mesh final
{
public:
	Mesh() = default;
	Mesh(
		std::vector<MeshVertex>&& vertices,
		std::vector<unsigned int>&& indices,
		std::unordered_map<TextureType, TextureFile>&& textures);
	
	void Delete();

	void AddTextureIfEmpty(TextureType tType, const std::string& filePath);

	// Render the mesh
	void Draw(GLuint shaderProgram, bool skipTexture) const;

private:
	// Initializes all the buffer objects/arrays
	void setupMesh();

	// Mesh Data
	std::vector<MeshVertex>	  m_vertices{};
	std::vector<unsigned int> m_indices{};
	std::unordered_map<TextureType, TextureFile> m_textureMap{};

	// Render data 
	gl4::VertexArray m_vao{0}; // TODO: возможно vao должно быть одно для всех моделей, а буферы размещать за счет glVertexArrayVertexBuffer/glVertexArrayElementBuffer
	gl4::Buffer m_vbo{0};
	gl4::Buffer m_ibo{0};
};