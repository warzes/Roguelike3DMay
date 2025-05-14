#pragma once

class TestSimple final : public IEngineApp
{
public:
	TestSimple() = default;
	TestSimple(const TestSimple&) = delete;
	TestSimple(TestSimple&&) = delete;
	void operator=(const TestSimple&) = delete;
	void operator=(TestSimple&&) = delete;

	EngineConfig GetConfig() const final;

	bool OnCreate() final;
	void OnDestroy() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;
};