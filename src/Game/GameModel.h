#pragma once

Mesh* LoadDataMesh(const std::vector<MeshVertex>& vertex, const std::vector<uint32_t>& indices);
Mesh* LoadAssimpMesh(const std::string& filename);

struct GameModel final
{
	Mesh* mesh{ nullptr };

	PhongMaterial material;
	gl4::MagFilter textureFilter{};

	glm::vec3 position{ 0.0f };
	glm::vec3 rotation{ 0.0f }; // в градусах // TODO: заменить на кватернион?
	glm::vec3 scale{ 1.0f };
	glm::mat4 GetModelMat() const;
};