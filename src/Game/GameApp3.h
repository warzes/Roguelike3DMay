#pragma once

#include "GameModel.h"
#include "UniformObjects.h"

class GameApp3 final : public IEngineApp
{
public:
	GameApp3();
	GameApp3(const GameApp3&) = delete;
	GameApp3(GameApp3&&) = delete;
	void operator=(const GameApp3&) = delete;
	void operator=(GameApp3&&) = delete;

	EngineCreateInfo GetCreateInfo() const final;

	bool OnInit() final;
	void OnClose() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;
	void OnMouseButton(int button, int action, int mods) final;
	void OnMousePos(double x, double y) final;
	void OnScroll(double dx, double dy) final;
	void OnKey(int key, int scanCode, int action, int mods) final;

	Camera& GetCamera() { return m_camera; }

private:
	bool createPipeline();
	void drawModel(GameModelOld& model);
	void drawModel(std::optional<GameModel> model);

	std::optional<gl::Texture> m_finalColorBuffer;
	std::optional<gl::Texture> m_finalDepthBuffer;

	Camera                      m_camera;
	glm::mat4                   m_projection;

	GameModelOld m_model1;
	GameModelOld m_model2;

	std::optional<GameModel> m_model3;

	std::optional<gl::GraphicsPipeline>              m_pipeline;
	std::optional<gl::TypedBuffer<GlobalUniforms>>   m_globalUbo;
	GlobalUniforms                                    m_globalUboData;
	std::optional<gl::TypedBuffer<ObjectUniforms>>   m_objectUbo;
	ObjectUniforms                                    m_objectUboData;
	std::optional<gl::TypedBuffer<MaterialUniforms>> m_materialUbo;
	MaterialUniforms                                  m_materialUboData;

	std::optional<gl::Sampler>                       m_nearestSampler;
	std::optional<gl::Sampler>                       m_linearSampler;
};