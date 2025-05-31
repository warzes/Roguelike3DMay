#pragma once

class GameAppOld final : public IEngineApp
{
public:
	GameAppOld() = default;
	GameAppOld(const GameAppOld&) = delete;
	GameAppOld(GameAppOld&&) = delete;
	void operator=(const GameAppOld&) = delete;
	void operator=(GameAppOld&&) = delete;

	EngineConfig GetConfig() const final;

	bool OnCreate() final;
	void OnDestroy() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;
};