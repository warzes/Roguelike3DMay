#pragma once

class TestShadowMapping final : public IEngineApp
{
public:
	TestShadowMapping() = default;
	TestShadowMapping(const TestShadowMapping&) = delete;
	TestShadowMapping(TestShadowMapping&&) = delete;
	void operator=(const TestShadowMapping&) = delete;
	void operator=(TestShadowMapping&&) = delete;

	EngineCreateInfo GetCreateInfo() const final;

	bool OnInit() final;
	void OnClose() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;

	void RenderScene(gl4::ShaderProgramId shader, int modelMatLoc);
};