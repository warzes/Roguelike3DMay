#pragma once

class TestDeferredSSAO final : public IEngineApp
{
public:
	TestDeferredSSAO() = default;
	TestDeferredSSAO(const TestDeferredSSAO&) = delete;
	TestDeferredSSAO(TestDeferredSSAO&&) = delete;
	void operator=(const TestDeferredSSAO&) = delete;
	void operator=(TestDeferredSSAO&&) = delete;

	EngineCreateInfo GetCreateInfo() const final;

	bool OnInit() final;
	void OnClose() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;

	void InitScene();
	void UpdateLightPositions();
	void InitLights();
	void RenderLights();
	void RenderScene(gl::ShaderProgramId shader) const;

	void OnMouseButton(int button, int action, int mods) final {}
	void OnMousePos(double x, double y) final {}
	void OnScroll(double dx, double dy) final {}
	void OnKey(int key, int scanCode, int action, int mods) final {}

	GraphicSystem gr;
};