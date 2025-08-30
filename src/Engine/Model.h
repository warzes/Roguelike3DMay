#pragma once

#include "Mesh.h"

class Model final
{
public:
	~Model();

	bool Load(const std::string& fileName);
	void Create(const MeshCreateInfo& meshCreateInfo);
	void Create(const std::vector<MeshCreateInfo>& meshes);

	void Free();

	void DrawSubMesh(size_t id);
	void Draw();

	size_t GetNumMeshes() const { return m_meshes.size(); }
	const std::vector<Mesh*>& GetMeshes() const { return m_meshes; }
	Mesh* GetMesh(size_t id) { return m_meshes[id]; }
	Mesh* GetMesh(size_t id) const { return m_meshes[id]; }
	const AABB& GetAABB() const { return m_aabb; }

private:
	void processNode(const aiScene* scene, aiNode* node, std::string_view directory);
	Mesh* processMesh(const aiScene* scene, struct aiMesh* mesh, std::string_view directory);
	void computeAABB();

	std::vector<Mesh*> m_meshes;
	AABB               m_aabb;
};
