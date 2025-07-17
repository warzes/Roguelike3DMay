#pragma once

#include "GameModel.h"

constexpr size_t MaxModelDraw = 1'000'000;

class ShadowPassManager;

/*
Класс отвечающий за работу с моделями/мешами в рамках игры
*/

namespace modelUBO
{
	struct alignas(16) GlobalUniforms final
	{
		glm::mat4 view;
		glm::mat4 proj;
	};

	struct alignas(16) ObjectUniforms final
	{
		glm::mat4 model;
	};

	struct alignas(16) MaterialUniform final
	{
		glm::vec3 diffuseMaterial;
		float pag0;
		glm::vec3 specularMaterial;
		float shininessMaterial;

		int hasDiffuse;
		int hasSpecular;
		int hasEmission;
		int hasNormalMap;
		int hasDepthMap;
		float emissionStrength;

		int blinn{ true };

		float heightScale;
	};
}

class GameModelManager final
{
public:
	bool Init();
	void Close();

	void Update();

	void SetModel(GameModel* model);

	void Draw(Camera& cam, ShadowPassManager& shadowPassMgr);
	void DrawInDepth(Camera& cam, ShadowPassManager& shadowPassMgr);

private:
	bool createPipeline();
	std::optional<gl4::GraphicsPipeline>                      m_pipeline;
	std::optional<gl4::GraphicsPipeline>                      m_pipelineInDepth;
	std::optional<gl4::TypedBuffer<modelUBO::GlobalUniforms>> m_globalUniformsUbo;
	std::optional<gl4::TypedBuffer<modelUBO::ObjectUniforms>> m_objectUniformUbo;
	std::optional<gl4::TypedBuffer<modelUBO::MaterialUniform>> m_materialUniformUbo;

	std::optional<gl4::Sampler>                               m_nearestSampler;
	std::optional<gl4::Sampler>                               m_linearSampler;

	std::vector<GameModel*> m_models;
	size_t                  m_currentModel{ 0 };
	size_t                  m_currentDrawShadowModel{ 0 };
};