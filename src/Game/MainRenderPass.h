#pragma once

#include "UniformObjects.h"

class GameModelOld;
class World;

#define DEFAULT_NUM_SAMPLES 16
#define MIN_NUM_SAMPLES 4
#define MAX_NUM_SAMPLES 256

class MainRenderPass final
{
public:
	bool Init(World *world);
	void Close();

	void Begin(Camera& cam, const glm::mat4& proj);
	void DrawModel(GameModelOld& model);

private:
	bool createPipeline();

	World* m_world{ nullptr };

	std::optional<gl::GraphicsPipeline>              m_pipeline;
	std::optional<gl::TypedBuffer<GlobalUniforms>>   m_globalUbo;
	GlobalUniforms                                    m_globalUboData;
	std::optional<gl::TypedBuffer<ObjectUniforms>>   m_objectUbo;
	ObjectUniforms                                    m_objectUboData;
	std::optional<gl::TypedBuffer<MainFragmentUniforms>> m_mainFragUbo;
	MainFragmentUniforms                                  m_mainFragUboData;

	std::optional<gl::TypedBuffer<MaterialUniforms>> m_materialUbo;
	MaterialUniforms                                  m_materialUboData;

	std::optional<gl::Sampler>                       m_nearestSampler;
	std::optional<gl::Sampler>                       m_linearSampler;

	std::optional<gl::Buffer>                        m_lightSSBO;

	size_t m_numBlockerSearchSamples = DEFAULT_NUM_SAMPLES;
	size_t m_numPCFSamples = DEFAULT_NUM_SAMPLES;
	std::optional<gl::Texture> m_distributions0;
	std::optional<gl::Texture> m_distributions1;
	std::optional<gl::Sampler> m_distributionsSampler;
};