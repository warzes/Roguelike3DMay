#pragma once

class TestModelLoading final : public IEngineApp
{
public:
	TestModelLoading() = default;
	TestModelLoading(const TestModelLoading&) = delete;
	TestModelLoading(TestModelLoading&&) = delete;
	void operator=(const TestModelLoading&) = delete;
	void operator=(TestModelLoading&&) = delete;

	EngineCreateInfo GetCreateInfo() const final;

	bool OnInit() final;
	void OnClose() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;

	void OnMouseButton(int button, int action, int mods) final {}
	void OnMousePos(double x, double y) final {}
	void OnScroll(double dx, double dy) final {}
	void OnKey(int key, int scanCode, int action, int mods) final {}

	GraphicSystem gr;
};