#pragma once

//=============================================================================
// Вывод кубов на сцену с освещением (блин-фонг) без текстур
// - несколько UBO
//=============================================================================
class Example005 final : public IEngineApp
{
public:
	Example005() = default;
	Example005(const Example005&) = delete;
	Example005(Example005&&) = delete;
	void operator=(const Example005&) = delete;
	void operator=(Example005&&) = delete;

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
	std::optional<gl::Buffer>           m_matrixUBO;
	std::optional<gl::Buffer>           m_sceneUBO;
	std::optional<gl::Buffer>           m_materialUBO;
	std::optional<gl::Buffer>           m_lightUBO;
	std::optional<gl::GraphicsPipeline> m_pipeline;
	Camera                              m_camera;
};