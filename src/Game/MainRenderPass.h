#pragma once

#include "UniformObjects.h"

class GameModel;

class MainRenderPass final
{
public:
	bool Init();
	void Close();

	void Begin(Camera& cam);
	void DrawModel(GameModel& model);

private:
	bool createPipeline();

	std::optional<gl4::GraphicsPipeline>              m_pipeline;
	std::optional<gl4::TypedBuffer<GlobalUniforms>>   m_globalUbo;
	GlobalUniforms                                    m_globalUboData;
	std::optional<gl4::TypedBuffer<ObjectUniforms>>   m_objectUbo;
	ObjectUniforms                                    m_objectUboData;
	std::optional<gl4::TypedBuffer<MaterialUniforms>> m_materialUbo;
	MaterialUniforms                                  m_materialUboData;

	std::optional<gl4::Sampler>                       m_nearestSampler;
	std::optional<gl4::Sampler>                       m_linearSampler;
};