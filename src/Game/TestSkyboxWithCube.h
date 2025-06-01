#pragma once

class TestSkyboxWithCube final : public IEngineApp
{
public:
	TestSkyboxWithCube() = default;
	TestSkyboxWithCube(const TestSkyboxWithCube&) = delete;
	TestSkyboxWithCube(TestSkyboxWithCube&&) = delete;
	void operator=(const TestSkyboxWithCube&) = delete;
	void operator=(TestSkyboxWithCube&&) = delete;

	EngineCreateInfo GetCreateInfo() const final;

	bool OnInit() final;
	void OnClose() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;
};