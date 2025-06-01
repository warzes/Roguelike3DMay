#pragma once

class NewTest001 final : public IEngineApp
{
public:
	NewTest001() = default;
	NewTest001(const NewTest001&) = delete;
	NewTest001(NewTest001&&) = delete;
	void operator=(const NewTest001&) = delete;
	void operator=(NewTest001&&) = delete;

	EngineCreateInfo GetCreateInfo() const final;

	bool OnInit() final;
	void OnClose() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;
};