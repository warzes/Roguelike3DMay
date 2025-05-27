#pragma once

class NewTest004 final : public IEngineApp
{
public:
	NewTest004() = default;
	NewTest004(const NewTest004&) = delete;
	NewTest004(NewTest004&&) = delete;
	void operator=(const NewTest004&) = delete;
	void operator=(NewTest004&&) = delete;

	EngineConfig GetConfig() const final;

	bool OnCreate() final;
	void OnDestroy() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;
};