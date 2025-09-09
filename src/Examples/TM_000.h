#pragma once

class TM_000 final : public IEngineApp
{
public:
	TM_000() = default;
	TM_000(const TM_000&) = delete;
	TM_000(TM_000&&) = delete;
	void operator=(const TM_000&) = delete;
	void operator=(TM_000&&) = delete;

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