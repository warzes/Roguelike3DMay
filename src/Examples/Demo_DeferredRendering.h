#pragma once

//=============================================================================
// DeferredRendering
//=============================================================================
class Demo_DeferredRendering final : public IEngineApp
{
public:
	Demo_DeferredRendering() = default;
	Demo_DeferredRendering(const Demo_DeferredRendering&) = delete;
	Demo_DeferredRendering(Demo_DeferredRendering&&) = delete;
	void operator=(const Demo_DeferredRendering&) = delete;
	void operator=(Demo_DeferredRendering&&) = delete;

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