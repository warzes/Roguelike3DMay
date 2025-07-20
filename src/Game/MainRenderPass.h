#pragma once

#include "UniformObjects.h"

class GameModel;

class MainRenderPass final
{
public:
	bool Init(const std::vector<Light>& lights);
	void Close();

	void Begin(const std::vector<Light>& lights, Camera& cam, const glm::mat4& proj);
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

	std::optional<gl4::Buffer>                        m_lightSSBO;
};