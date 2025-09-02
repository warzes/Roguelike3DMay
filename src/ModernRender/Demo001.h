#pragma once

#include "SceneManager.h"
#include "RenderPassManager.h"

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
	void sceneDraw();
	SceneManager      m_sceneManager;
	RenderPassManager m_renderPassManager;

	Camera            m_camera;

	Model             m_plane;
	Model             m_box;
	Model             m_sphere;
	Model             m_house;

	gl::Texture* m_texture1;
	gl::Texture* m_texture2;
	std::optional<gl::Sampler> m_sampler;
};