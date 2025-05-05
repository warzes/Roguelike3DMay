#pragma once

void ExitApp();

struct EngineConfig final
{
	struct Window final
	{
		uint16_t         width{ 1600 };
		uint16_t         height{ 900 };
		std::string_view title{ "Game" };
	} window;

	struct Render final
	{
		bool             vsync{ true };
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
	
	float GetDeltaTime() const { return m_deltaTime; }

private:
	friend void framebufferSizeCallback(GLFWwindow*, int, int) noexcept;

	bool create();
	bool createWindow(const EngineConfig& config);
	void initOpenGL();
	void initImGui();
	bool shouldWindowClose() const;
	void destroy();

	void windowResize(int width, int height);

	GLFWwindow* m_window;
	uint16_t m_width{ 0 };
	uint16_t m_height{ 0 };
	float m_deltaTime{ 0.0f };
};