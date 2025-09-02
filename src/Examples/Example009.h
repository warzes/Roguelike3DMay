#pragma once

//=============================================================================
// Вывод простой сцены в фреймбуфер
// - сцена рендерится в текстуру (цвет и глубина)
// - затем рисуется квад на SwapChain с этой текстурой
// TODO: есть еще блитинг текстуры на поверхность, сделать по нему пример
//		gl::BlitTextureToSwapChain(texture, {}, {}, { texture.width, texture.height, 1 }, { GetWindowWidth(), GetWindowHeight(), 1 }, gl::MagFilter::Nearest);
//=============================================================================
class Example009 final : public IEngineApp
{
public:
	Example009() = default;
	Example009(const Example009&) = delete;
	Example009(Example009&&) = delete;
	void operator=(const Example009&) = delete;
	void operator=(Example009&&) = delete;

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
	std::optional<gl::Buffer>           m_uniformBuffer;
	std::optional<gl::GraphicsPipeline> m_scenePipeline;
	std::optional<gl::Buffer>           m_quadvb;
	std::optional<gl::Buffer>           m_quadib;
	std::optional<gl::GraphicsPipeline> m_finalPipeline;
	gl::Texture*                        m_texture1{ nullptr };
	gl::Texture*                        m_texture2{ nullptr };
	std::optional<gl::Sampler>          m_sampler;
	Camera                              m_camera;

	std::optional<gl::Texture>          m_fboColorTex;
	std::optional<gl::Texture>          m_fboDepthTex;

};