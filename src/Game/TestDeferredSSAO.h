#pragma once

class TestDeferredSSAO final : public IEngineApp
{
public:
	TestDeferredSSAO() = default;
	TestDeferredSSAO(const TestDeferredSSAO&) = delete;
	TestDeferredSSAO(TestDeferredSSAO&&) = delete;
	void operator=(const TestDeferredSSAO&) = delete;
	void operator=(TestDeferredSSAO&&) = delete;

	EngineCreateInfo GetCreateInfo() const final;

	bool OnInit() final;
	void OnClose() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;

	void InitScene();
	void UpdateLightPositions();
	void InitLights();
	void RenderLights();
	void RenderScene(gl4::ShaderProgramId shader) const;
};