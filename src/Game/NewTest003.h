#pragma once

class NewTest003 final : public IEngineApp
{
public:
	NewTest003() = default;
	NewTest003(const NewTest003&) = delete;
	NewTest003(NewTest003&&) = delete;
	void operator=(const NewTest003&) = delete;
	void operator=(NewTest003&&) = delete;

	EngineCreateInfo GetCreateInfo() const final;

	bool OnInit() final;
	void OnClose() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;
	void OnMouseButton(int button, int action, int mods) final;
	void OnMousePos(double x, double y) final;
	void OnScroll(double dx, double dy) final;
	void OnKey(int key, int scanCode, int action, int mods) final;
};