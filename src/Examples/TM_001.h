#pragma once

class TM_001 final : public IEngineApp
{
public:
	TM_001() = default;
	TM_001(const TM_001&) = delete;
	TM_001(TM_001&&) = delete;
	void operator=(const TM_001&) = delete;
	void operator=(TM_001&&) = delete;

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