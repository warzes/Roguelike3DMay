#pragma once

#include "TestComplex_Temp.h"

class TestComplex final : public IEngineApp
{
public:
	TestComplex() = default;
	TestComplex(const TestComplex&) = delete;
	TestComplex(TestComplex&&) = delete;
	void operator=(const TestComplex&) = delete;
	void operator=(TestComplex&&) = delete;

	EngineCreateInfo GetCreateInfo() const final;

	bool OnInit() final;
	void OnClose() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;
};