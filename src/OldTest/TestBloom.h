#pragma once

class TestBloom final : public IEngineApp
{
public:
	TestBloom() = default;
	TestBloom(const TestBloom&) = delete;
	TestBloom(TestBloom&&) = delete;
	void operator=(const TestBloom&) = delete;
	void operator=(TestBloom&&) = delete;

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