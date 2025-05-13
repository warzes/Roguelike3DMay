#pragma once

class TestBloom final : public IEngineApp
{
public:
	TestBloom() = default;
	TestBloom(const TestBloom&) = delete;
	TestBloom(TestBloom&&) = delete;
	void operator=(const TestBloom&) = delete;
	void operator=(TestBloom&&) = delete;

	EngineConfig GetConfig() const final;

	bool OnCreate() final;
	void OnDestroy() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;
};