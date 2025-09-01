#pragma once

//=============================================================================
// Вывод простой сцены с прозрачностью
// - класс Model и создание плоскости и кубов
// - используется базовый формат вершины для мешей MeshVertexInputBindingDescs
//=============================================================================
class Example008 final : public IEngineApp
{
public:
	Example008() = default;
	Example008(const Example008&) = delete;
	Example008(Example008&&) = delete;
	void operator=(const Example008&) = delete;
	void operator=(Example008&&) = delete;

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
	Model                               m_cube;
	Model                               m_plane;
	Model                               m_window;
	std::optional<gl::Buffer>           m_uniformBuffer;
	std::optional<gl::GraphicsPipeline> m_pipeline;
	gl::Texture*                        m_texture1{ nullptr };
	gl::Texture*                        m_texture2{ nullptr };
	gl::Texture*                        m_texture3{ nullptr };
	std::optional<gl::Sampler>          m_sampler;
	Camera                              m_camera;
};