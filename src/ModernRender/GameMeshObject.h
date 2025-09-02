#pragma once

#include "GameObject.h"

// TODO: установка материала для Create
// TODO: в Load возможно кеширование по типу текстурменеджера, чтобы грузился один меш

class GameMeshObject final : public GameObject
{
public:
	GameMeshObject();

	bool Load(const std::string& fileName, std::optional<glm::mat4> modelTransformMatrix = std::nullopt);
	void Create(const MeshCreateInfo& meshCreateInfo);
	void Create(const std::vector<MeshCreateInfo>& meshes);

	void Free();

	// TODO: установка модельной матрицы в UBO
	void Draw(std::optional<gl::Sampler> sampler);
private:
	Model m_model;
};