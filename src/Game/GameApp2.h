#pragma once

#include "World.h"
#include "WorldRender.h"

class GameApp2 final : public IEngineApp
{
public:
	GameApp2();
	GameApp2(const GameApp2&) = delete;
	GameApp2(GameApp2&&) = delete;
	void operator=(const GameApp2&) = delete;
	void operator=(GameApp2&&) = delete;

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
	World                       m_world;
	WorldRender                 m_renderWorld;
	Camera                      m_camera;
	glm::mat4                   m_projection;

	std::optional<gl4::Texture> m_finalColorBuffer;
	std::optional<gl4::Texture> m_finalDepthBuffer;
};