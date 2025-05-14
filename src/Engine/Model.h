#pragma once

#include "Mesh.h"

#define ASSIMP_LOAD_FLAGS (aiProcess_JoinIdenticalVertices |    \
                           aiProcess_Triangulate |              \
                           aiProcess_GenSmoothNormals |         \
                           aiProcess_LimitBoneWeights |         \
                           aiProcess_SplitLargeMeshes |         \
                           aiProcess_ImproveCacheLocality |     \
                           aiProcess_RemoveRedundantMaterials | \
                           aiProcess_FindDegenerates |          \
                           aiProcess_FindInvalidData |          \
                           aiProcess_GenUVCoords |              \
                           aiProcess_FlipUVs |                  \
                           aiProcess_MakeLeftHanded |           \
                           aiProcess_CalcTangentSpace)

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