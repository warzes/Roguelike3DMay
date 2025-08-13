#pragma once

class TestForwardPlus final : public IEngineApp
{
public:
	TestForwardPlus() = default;
	TestForwardPlus(const TestForwardPlus&) = delete;
	TestForwardPlus(TestForwardPlus&&) = delete;
	void operator=(const TestForwardPlus&) = delete;
	void operator=(TestForwardPlus&&) = delete;

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
};