#pragma once

#include "UniformObjects.h"

class GameModel;

class ShadowPass final
{
public:
	bool Init();
	void Close();

	void Begin();
	void DrawModel(GameModel& model);
	void End();

	void BindShadowMap(uint32_t index);

	gl4::Texture*                      depthTexture{ nullptr };
	gl4::RenderInfo*                   viewport{ nullptr };
	gl4::RenderDepthStencilAttachment* rtAttachment{ nullptr };
	uint32_t                           width{ 4096 };
	uint32_t                           height{ 4096 };
	glm::mat4 lightProjection;
	glm::mat4 lightView;
	glm::vec3 shadowLightPos;
private:
	bool createPipeline();

	std::optional<gl4::GraphicsPipeline>              m_pipeline;
	std::optional<gl4::TypedBuffer<GlobalUniforms>>   m_globalUbo;
	GlobalUniforms                                    m_globalUboData;
	std::optional<gl4::TypedBuffer<ObjectUniforms>>   m_objectUbo;
	ObjectUniforms                                    m_objectUboData;

	std::optional<gl4::Sampler>                       m_linearSampler;
};