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

	void RenderScene(gl::ShaderProgramId shader);

	void OnMouseButton(int button, int action, int mods) final {}
	void OnMousePos(double x, double y) final {}
	void OnScroll(double dx, double dy) final {}
	void OnKey(int key, int scanCode, int action, int mods) final {}

	GraphicSystem gr;
};