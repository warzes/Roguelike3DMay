#pragma once

class TestShadowMapping final : public IEngineApp
{
public:
	TestShadowMapping() = default;
	TestShadowMapping(const TestShadowMapping&) = delete;
	TestShadowMapping(TestShadowMapping&&) = delete;
	void operator=(const TestShadowMapping&) = delete;
	void operator=(TestShadowMapping&&) = delete;

	EngineCreateInfo GetCreateInfo() const final;

	bool OnInit() final;
	void OnClose() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;

	void RenderScene(gl::ShaderProgramId shader, int modelMatLoc);


	void OnMouseButton(int button, int action, int mods) final {}
	void OnMousePos(double x, double y) final {}
	void OnScroll(double dx, double dy) final {}
	void OnKey(int key, int scanCode, int action, int mods) final {}
};