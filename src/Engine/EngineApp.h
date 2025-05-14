#pragma once

#include "GraphicSystem.h"

void ExitApp();

struct EngineConfig final
{
	struct Window final
	{
		uint16_t         width{ 1600 };
		uint16_t         height{ 900 };
		std::string_view title{ "Game" };
		bool             maximized{ false };
		bool             fullScreen{ false };
	} window;

	struct Render final
	{
		bool             vsync{ false };
		bool             srgb{ false };
	} render;
};

constexpr size_t MaxKeys = 512;
constexpr size_t MaxMouseButtons = 5;

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

	virtual EngineConfig GetConfig() const = 0;
	virtual bool OnCreate() = 0;
	virtual void OnDestroy() = 0;
	virtual void OnUpdate(float deltaTime) = 0;
	virtual void OnRender() = 0;
	virtual void OnImGuiDraw() = 0;

	virtual void OnResize(uint16_t width, uint16_t height) = 0;

	GLFWwindow* GetGLFWWindow() { return m_window; }
	uint16_t GetWidth() const { return m_width; }
	uint16_t GetHeight() const { return m_height; }
	float GetAspect() const;

	int GetMousePositionX() const { return static_cast<int>(m_currentMousePositionX); }
	int GetMousePositionY() const { return static_cast<int>(m_currentMousePositionY); }
	glm::ivec2 GetMousePosition() const { return { GetMousePositionX(), GetMousePositionY() }; }

	int GetMouseDeltaX() const { return static_cast<int>(m_mouseDeltaX); }
	int GetMouseDeltaY() const { return static_cast<int>(m_mouseDeltaY); }
	glm::ivec2 GetMouseDelta() const { return { GetMouseDeltaX(), GetMouseDeltaY() }; }

	bool GetKeyDown(int key);
	bool GetKeyPressed(int key);
	bool GetMouseButton(int button);
	
	float GetDeltaTime() const { return m_deltaTime; }
	unsigned GetFPS() const { return m_fps; }

	double GetTimeInSec() const;

	void SetCursorPosition(const glm::uvec2& position);

	void DrawProfilerInfo();

	GraphicSystem& GetGraphicSystem() { return m_graphics; }

	void SetCursorVisible(bool visible);

private:
	friend void framebufferSizeCallback(GLFWwindow*, int, int) noexcept;
	friend void keyCallback(GLFWwindow*, int, int, int, int) noexcept;
	friend void mouseButtonCallback(GLFWwindow*, int, int, int) noexcept;
	friend void mouseCursorPosCallback(GLFWwindow*, double, double) noexcept;

	bool create();
	bool createWindow(const EngineConfig& config);
	void initOpenGL();
	void initImGui();
	bool shouldWindowClose() const;
	void destroy();

	void windowResize(int width, int height);

	GLFWwindow* m_window;
	uint16_t    m_width{ 0 };
	uint16_t    m_height{ 0 };
	float       m_deltaTime{ 0.0f };
	unsigned    m_fps{ 0 };

	double      m_currentMousePositionX{ 0.0 };
	double      m_currentMousePositionY{ 0.0 };
	double      m_mouseLastX{ 0.0 };
	double      m_mouseLastY{ 0.0 };
	double      m_mouseDeltaX{ 0.0 };
	double      m_mouseDeltaY{ 0.0 };
	bool        m_cursorVisible{ true };

	std::array<bool, MaxKeys> m_keys{ false };
	std::array<bool, MaxKeys> m_repeatKeys{ false };
	std::array<bool, MaxMouseButtons> m_mouseButtons{ false };

	GraphicSystem m_graphics;
};

uint16_t GetWindowWidth();
uint16_t GetWindowHeight();
float GetWindowAspect();