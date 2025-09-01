#pragma once

//=============================================================================
// Вывод кубов на сцену и движение по ней с помощью камеры
// - вывод кубов. куб с нормалью
// - включения Z буфера
// - Camera
//=============================================================================
class Example004 final : public IEngineApp
{
public:
	Example004() = default;
	Example004(const Example004&) = delete;
	Example004(Example004&&) = delete;
	void operator=(const Example004&) = delete;
	void operator=(Example004&&) = delete;

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
	std::optional<gl::Buffer>           m_uniformBuffer;
	std::optional<gl::GraphicsPipeline> m_pipeline;
	gl::Texture*                        m_texture{ nullptr };
	std::optional<gl::Sampler>          m_sampler;
	Camera                              m_camera;
};