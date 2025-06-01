#pragma once

class TestCascadedShadowMaps final : public IEngineApp
{
public:
	TestCascadedShadowMaps() = default;
	TestCascadedShadowMaps(const TestCascadedShadowMaps&) = delete;
	TestCascadedShadowMaps(TestCascadedShadowMaps&&) = delete;
	void operator=(const TestCascadedShadowMaps&) = delete;
	void operator=(TestCascadedShadowMaps&&) = delete;

	EngineCreateInfo GetCreateInfo() const final;

	bool OnInit() final;
	void OnClose() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;
};