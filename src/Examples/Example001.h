#pragma once

//=============================================================================
// Вывод треугольника на основную поверхность
// - вершинный буфер из позиции и цвета
// - индексный буфер
// - создание GraphicsPipeline
// - дополнительное окно imGui с информацией об GAPI
//=============================================================================
class Example001 final : public IEngineApp
{
public:
	Example001() = default;
	Example001(const Example001&) = delete;
	Example001(Example001&&) = delete;
	void operator=(const Example001&) = delete;
	void operator=(Example001&&) = delete;

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
	std::optional<gl::Buffer>           m_vertexBuffer;
	std::optional<gl::Buffer>           m_indexBuffer;
	std::optional<gl::GraphicsPipeline> m_pipeline;
};