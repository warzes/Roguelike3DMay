#pragma once

class TestTerrain final : public IEngineApp
{
public:
	TestTerrain() = default;
	TestTerrain(const TestTerrain&) = delete;
	TestTerrain(TestTerrain&&) = delete;
	void operator=(const TestTerrain&) = delete;
	void operator=(TestTerrain&&) = delete;

	EngineCreateInfo GetCreateInfo() const final;

	bool OnInit() final;
	void OnClose() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;
};