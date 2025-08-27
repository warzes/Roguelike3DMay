#pragma once

#include "Material.h"

Mesh* LoadDataMesh(const std::vector<MeshVertex>& vertex, const std::vector<uint32_t>& indices, PhongMaterial* material = nullptr);
Mesh* LoadAssimpMesh(const std::string& filename);

struct GameModelOld final
{
	Mesh* mesh{ nullptr };

	PhongMaterial  material;
	gl::MagFilter textureFilter{};

	glm::vec3 position{ 0.0f };
	glm::vec3 rotation{ 0.0f }; // в градусах // TODO: заменить на кватернион?
	glm::vec3 scale{ 1.0f };

	glm::mat4 GetModelMat() const;
};

struct GameModel final
{
	void Free();

	std::vector<Mesh*> meshes;
	gl::MagFilter textureFilter{};

	AABB      aabb;
	glm::vec3 position{ 0.0f };
	glm::vec3 rotation{ 0.0f }; // в градусах // TODO: заменить на кватернион?
	glm::vec3 scale{ 1.0f };

	glm::mat4 GetModelMat() const;
};

std::optional<GameModel> LoadAssimpModel(const std::string& filename);