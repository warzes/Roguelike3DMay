#pragma once

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
};