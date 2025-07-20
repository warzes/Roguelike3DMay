#pragma once

#include "UniformObjects.h"

class GameModel;

class MainRenderPass final
{
public:
	bool Init();
	void Close();

	void SetState(Camera& cam, GameModel& model);

private:
	bool createPipeline();

	std::optional<gl4::GraphicsPipeline>            m_pipeline;
	std::optional<gl4::TypedBuffer<GlobalUniforms>> m_globalUbo;
	std::optional<gl4::TypedBuffer<ObjectUniforms>> m_objectUbo;
	std::optional<gl4::Sampler>                     m_nearestSampler;
	std::optional<gl4::Sampler>                     m_linearSampler;
};