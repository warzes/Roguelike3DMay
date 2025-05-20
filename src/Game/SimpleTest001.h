#pragma once

class SimpleTest001 final : public IEngineApp
{
public:
	SimpleTest001() = default;
	SimpleTest001(const SimpleTest001&) = delete;
	SimpleTest001(SimpleTest001&&) = delete;
	void operator=(const SimpleTest001&) = delete;
	void operator=(SimpleTest001&&) = delete;

	EngineConfig GetConfig() const final;

	bool OnCreate() final;
	void OnDestroy() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;
};