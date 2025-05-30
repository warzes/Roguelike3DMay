#pragma once

class NewTest002 final : public IEngineApp
{
public:
	NewTest002() = default;
	NewTest002(const NewTest002&) = delete;
	NewTest002(NewTest002&&) = delete;
	void operator=(const NewTest002&) = delete;
	void operator=(NewTest002&&) = delete;

	EngineConfig GetConfig() const final;

	bool OnCreate() final;
	void OnDestroy() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;
};