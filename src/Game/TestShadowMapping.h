#pragma once

class TestShadowMapping final : public IEngineApp
{
public:
	TestShadowMapping() = default;
	TestShadowMapping(const TestShadowMapping&) = delete;
	TestShadowMapping(TestShadowMapping&&) = delete;
	void operator=(const TestShadowMapping&) = delete;
	void operator=(TestShadowMapping&&) = delete;

	EngineConfig GetConfig() const final;

	bool OnCreate() final;
	void OnDestroy() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;

	void RenderScene(GLuint shader, int modelMatLoc);
};