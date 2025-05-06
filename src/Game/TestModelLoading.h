#pragma once

class TestModelLoading final : public IEngineApp
{
public:
	TestModelLoading() = default;
	TestModelLoading(const TestModelLoading&) = delete;
	TestModelLoading(TestModelLoading&&) = delete;
	void operator=(const TestModelLoading&) = delete;
	void operator=(TestModelLoading&&) = delete;

	EngineConfig GetConfig() const final;

	bool OnCreate() final;
	void OnDestroy() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;
};