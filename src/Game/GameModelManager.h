#pragma once

#include "GameModel.h"

constexpr size_t MaxModelDraw = 1'000'000;

/*
Класс отвечающий за работу с моделями/мешами в рамках игры
*/

namespace modelUBO
{
	struct GlobalUniforms final
	{
		glm::mat4 view;
		glm::mat4 proj;
	};

	struct ObjectUniforms final
	{
		glm::mat4 model;
	};
}

class GameModelManager final
{
public:
	bool Init();
	void Close();

	void Update(Camera& cam);

	void SetModel(GameModel* model);

	void Draw();

private:
	bool createPipeline();
	std::optional<gl4::GraphicsPipeline>                      m_pipeline;
	std::optional<gl4::TypedBuffer<modelUBO::GlobalUniforms>> m_globalUniformsUbo;
	std::optional<gl4::TypedBuffer<modelUBO::ObjectUniforms>> m_objectUniformUbo;

	std::optional<gl4::Sampler>                               m_nearestSampler;
	std::optional<gl4::Sampler>                               m_linearSampler;

	std::vector<GameModel*> m_models;
	size_t                  m_currentModel{ 0 };
};