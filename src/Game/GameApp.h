#pragma once

class GameApp final : public IEngineApp
{
public:
	GameApp() = default;
	GameApp(const GameApp&) = delete;
	GameApp(GameApp&&) = delete;
	void operator=(const GameApp&) = delete;
	void operator=(GameApp&&) = delete;

	EngineConfig GetConfig() const final;

	bool OnCreate() final;
	void OnDestroy() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;
};