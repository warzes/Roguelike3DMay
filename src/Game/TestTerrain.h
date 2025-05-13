#pragma once

class TestTerrain final : public IEngineApp
{
public:
	TestTerrain() = default;
	TestTerrain(const TestTerrain&) = delete;
	TestTerrain(TestTerrain&&) = delete;
	void operator=(const TestTerrain&) = delete;
	void operator=(TestTerrain&&) = delete;

	EngineConfig GetConfig() const final;

	bool OnCreate() final;
	void OnDestroy() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;
};