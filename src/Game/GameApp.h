#pragma once

#include "GameModelManager.h"
#include "GameSceneManager.h"

class GameApp final : public IEngineApp
{
public:
	GameApp() = default;
	GameApp(const GameApp&) = delete;
	GameApp(GameApp&&) = delete;
	void operator=(const GameApp&) = delete;
	void operator=(GameApp&&) = delete;

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
	GameSceneManager m_scene;

	Camera           m_camera;

	std::optional<gl4::Texture> m_colorBuffer;
	std::optional<gl4::Texture> m_depthBuffer;

	GameModel        m_model1;
	GameModel        m_model2;
	GameModel        m_model3;
};