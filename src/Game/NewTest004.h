﻿#pragma once

class NewTest004 final : public IEngineApp
{
public:
	NewTest004() = default;
	NewTest004(const NewTest004&) = delete;
	NewTest004(NewTest004&&) = delete;
	void operator=(const NewTest004&) = delete;
	void operator=(NewTest004&&) = delete;

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
};