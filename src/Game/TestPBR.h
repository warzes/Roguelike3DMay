#pragma once

class TestPBR final : public IEngineApp
{
public:
	TestPBR() = default;
	TestPBR(const TestPBR&) = delete;
	TestPBR(TestPBR&&) = delete;
	void operator=(const TestPBR&) = delete;
	void operator=(TestPBR&&) = delete;

	EngineCreateInfo GetCreateInfo() const final;

	bool OnInit() final;
	void OnClose() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;

	void RenderScene(gl4::ShaderProgramId shader);
};