#pragma once

#include "SceneManager.h"

class Demo001 final : public IEngineApp
{
public:
	Demo001() = default;
	Demo001(const Demo001&) = delete;
	Demo001(Demo001&&) = delete;
	void operator=(const Demo001&) = delete;
	void operator=(Demo001&&) = delete;

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
	SceneManager m_sceneManager;

	RenderPass                          m_renderPass;

	std::optional<gl::Buffer>           m_quadvb;
	std::optional<gl::Buffer>           m_quadib;
	std::optional<gl::GraphicsPipeline> m_swapChainPipeline;
};