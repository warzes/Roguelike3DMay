#pragma once

//=============================================================================
// Вывод прямоугольника с текстурой на основную поверхность
// - текстура
// - семплер
//=============================================================================
class Example002 final : public IEngineApp
{
public:
	Example002() = default;
	Example002(const Example002&) = delete;
	Example002(Example002&&) = delete;
	void operator=(const Example002&) = delete;
	void operator=(Example002&&) = delete;

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

private:

};