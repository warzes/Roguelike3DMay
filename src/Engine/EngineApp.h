#pragma once

#include "GraphicSystem.h"

void ExitApp();

uint16_t GetWindowWidth();
uint16_t GetWindowHeight();
float GetWindowAspect();

bool GetKeyDown(int key);
bool GetMouseButton(int button);
int GetMousePositionX();
int GetMousePositionY();

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

constexpr inline size_t MaxKeys{ 512 };
constexpr inline size_t MaxMouseButtons{ 5 };

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

	double GetTimeInSec() const;

	int GetMousePositionX() const { return static_cast<int>(m_currentMousePositionX); }
	int GetMousePositionY() const { return static_cast<int>(m_currentMousePositionY); }
	glm::ivec2 GetMousePosition() const { return { GetMousePositionX(), GetMousePositionY() }; }
	int GetMouseDeltaX() const { return static_cast<int>(m_mouseDeltaX); }
	int GetMouseDeltaY() const { return static_cast<int>(m_mouseDeltaY); }
	glm::ivec2 GetMouseDelta() const { return { GetMouseDeltaX(), GetMouseDeltaY() }; }
	void SetCursorPosition(const glm::uvec2& position);
	void SetCursorVisible(bool visible);

	bool GetMouseButton(int button);
	bool GetKeyDown(int key);
	bool GetKeyPressed(int key);

	GLFWwindow* GetGLFWWindow() { return m_window; }
	uint16_t GetWindowWidth() const { return m_width; }
	uint16_t GetWindowHeight() const { return m_height; }
	float GetWindowAspect() const { return m_windowAspect; }
	
	float GetDeltaTime() const { return m_deltaTime; }
	unsigned GetFPS() const { return m_currentFPS; }

	void DrawProfilerInfo();
	void DrawFPS();

	GraphicSystem& GetGraphicSystem() { return m_graphics; }

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

	// system
	GraphicSystem m_graphics;

	// window config
	GLFWwindow* m_window{ nullptr };
	glm::ivec2  m_windowPosition{ 0 };
	uint16_t    m_width{ 0 };
	uint16_t    m_height{ 0 };
	float       m_windowAspect{ 1.0f };

	// time config
	float       m_deltaTime{ 0.0f };

	// fps
	const float m_avgInterval{ 0.5f };
	unsigned    m_numFrames{ 0 };
	double      m_accumulatedTime{ 0.0 };
	float       m_currentFPS{ 0.0f };

	// input config
	double      m_currentMousePositionX{ 0.0 };
	double      m_currentMousePositionY{ 0.0 };
	double      m_mouseLastX{ 0.0 };
	double      m_mouseLastY{ 0.0 };
	double      m_mouseDeltaX{ 0.0 };
	double      m_mouseDeltaY{ 0.0 };
	bool        m_cursorVisible{ true };
	std::array<bool, MaxKeys> m_keys{ { false } };
	std::array<bool, MaxKeys> m_repeatKeys{ { false } };
	std::array<bool, MaxMouseButtons> m_mouseButtons{ { false } };

	// state
	bool        m_canRender{ true };
};