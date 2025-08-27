#pragma once

#include "Input.h"

void ExitApp();

uint16_t GetWindowWidth();
uint16_t GetWindowHeight();
float GetWindowAspect();

struct EngineCreateInfo final
{
	struct Window final
	{
		uint16_t         width{ 1600 };
		uint16_t         height{ 900 };
		std::string_view title{ "Game" };
		bool             resizable{ true };
		bool             maximized{ false };
		bool             decorate{ true };
		bool             fullScreen{ false };
	} window;

	struct Render final
	{
		bool             vsync{ false };
		bool             srgb{ false };
	} render;
};

class IEngineApp
{
public:
	IEngineApp() = default;
	virtual ~IEngineApp() = default;

	IEngineApp(const IEngineApp&) = delete;
	IEngineApp(IEngineApp&&) = delete;
	void operator=(const IEngineApp&) = delete;
	void operator=(IEngineApp&&) = delete;

	void Run();

	void Exit();

	void DrawProfilerInfo();
	void DrawFPS();

	double GetTimeInSec() const;

	auto GetDeltaTime() const { return m_deltaTime; }
	auto GetCurrentTime() const { return m_currentTime; }
	auto GetFramesPerSecond() const { return m_framesPerSecond; }

	GLFWwindow* GetGLFWWindow() { return m_window; }
	uint16_t GetWindowWidth() const { return m_width; }
	uint16_t GetWindowHeight() const { return m_height; }
	float GetWindowAspect() const { return m_windowAspect; }

protected:
	virtual EngineCreateInfo GetCreateInfo() const = 0;
	virtual bool OnInit() = 0;
	virtual void OnClose() = 0;
	virtual void OnUpdate(float deltaTime) = 0;
	virtual void OnRender() = 0;
	virtual void OnImGuiDraw() = 0;
	virtual void OnResize(uint16_t width, uint16_t height) = 0;
	virtual void OnMouseButton(int button, int action, int mods) = 0;
	virtual void OnMousePos(double x, double y) = 0;
	virtual void OnScroll(double dx, double dy) = 0;
	virtual void OnKey(int key, int scanCode, int action, int mods) = 0;

private:
	friend void windowPosCallback(GLFWwindow*, int, int) noexcept;
	friend void framebufferSizeCallback(GLFWwindow*, int, int) noexcept;
	friend void keyCallback(GLFWwindow*, int, int, int, int) noexcept;
	friend void mouseButtonCallback(GLFWwindow*, int, int, int) noexcept;
	friend void mouseCursorPosCallback(GLFWwindow*, double, double) noexcept;
	friend void mouseScrollCallback(GLFWwindow*, double, double) noexcept;

	bool init();
	bool initWindow(const EngineCreateInfo& config);
	void initOpenGL();
	void initImGui();
	bool shouldWindowClose() const;
	void close();
	void windowResize(int width, int height);
	void fpsTick(float deltaSeconds, bool frameRendered = true);

	void keyPress(int key, int scanCode, int action, int mods);
	void mousePos(double xPos, double yPos);
	void mouseScroll(double xOffset, double yOffset);
	void mouseButton(int button, int action, int mods);

	// system

	// window config
	GLFWwindow* m_window{ nullptr };
	glm::ivec2  m_windowPosition{ 0 };
	uint16_t    m_width{ 0 };
	uint16_t    m_height{ 0 };
	float       m_windowAspect{ 1.0f };

	// time config
	float       m_deltaTime{ 0.0f };
	double      m_currentTime{ 0.0 };

	// fps
	const float m_avgInterval{ 0.5f };
	unsigned    m_frameCounter{ 0 };
	double      m_timeCounter{ 0.0 };
	float       m_framesPerSecond{ 0.0f };

	// state
	bool        m_canRender{ true };
};