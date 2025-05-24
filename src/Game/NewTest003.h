#pragma once

class NewTest003 final : public IEngineApp
{
public:
	NewTest003() = default;
	NewTest003(const NewTest003&) = delete;
	NewTest003(NewTest003&&) = delete;
	void operator=(const NewTest003&) = delete;
	void operator=(NewTest003&&) = delete;

	EngineConfig GetConfig() const final;

	bool OnCreate() final;
	void OnDestroy() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;
};