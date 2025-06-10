#pragma once

class NewTest005 final : public IEngineApp
{
public:
	NewTest005() = default;
	NewTest005(const NewTest005&) = delete;
	NewTest005(NewTest005&&) = delete;
	void operator=(const NewTest005&) = delete;
	void operator=(NewTest005&&) = delete;

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