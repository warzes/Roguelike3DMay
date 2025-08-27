#pragma once

#include "Mesh.h"

class Model final
{
public:
	~Model();

	bool Load(const std::string& fileName);
	void Free();

	size_t GetNumMeshes() const { return m_meshes.size(); }
	const std::vector<Mesh*>& GetMeshes() const { return m_meshes; }
	const AABB& GetAABB() const { return m_aabb; }

private:
	void processNode(const aiScene* scene, aiNode* node, std::string_view directory);
	Mesh* processMesh(const aiScene* scene, struct aiMesh* mesh, std::string_view directory);

	std::vector<Mesh*> m_meshes;
	AABB               m_aabb;
};
