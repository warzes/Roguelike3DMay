#pragma once

#include "UniformObjects.h"

class GameModelOld;
class World;

class ShadowPass final
{
public:
	bool Init(World* world);
	void Close();

	void Begin(const ShadowMap& shadow);
	void DrawModel(GameModelOld& model);
	void End();

	std::optional<gl4::Sampler> GetLinearSampler() { return m_linearSampler; }
private:
	bool createPipeline();

	World* m_world{ nullptr };

	std::optional<gl4::GraphicsPipeline>            m_pipeline;
	std::optional<gl4::TypedBuffer<ShadowUniforms>> m_ubo;
	ShadowUniforms                                  m_uboData;

	std::optional<gl4::Sampler>                     m_linearSampler;
};