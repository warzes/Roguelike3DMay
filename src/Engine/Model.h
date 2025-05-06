#pragma once

#include "Mesh.h"

class Model final
{
public:
	Model(const std::string& path);
	~Model();

	void Draw(GLuint shaderProgram, bool skipTexture = false) const;

	void AddTextureIfEmpty(TextureType tType, const std::string& filePath);

private:
	// Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void loadModel(const std::string& path);
	// Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void processNode(const aiNode* node, const aiScene* scene, const glm::mat4& parentTransform);
	Mesh processMesh(const aiMesh* mesh, const aiScene* scene, const glm::mat4& transform);

	// Model data 
	std::unordered_map<std::string, TextureFile> m_textureMap{}; // key is the filename
	std::vector<Mesh> m_meshes{};
	std::string m_directory{};
};