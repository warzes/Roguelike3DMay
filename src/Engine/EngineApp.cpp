#include "stdafx.h"
#include "EngineApp.h"
#include "Log.h"
#include "Profiler.h"
//=============================================================================
#if defined(_WIN32)
extern "C" {
	_declspec(dllexport) uint32_t NvOptimusEnablement = 1;
	_declspec(dllexport) uint32_t AmdPowerXpressRequestHighPerformance = 1;
}
#endif
//=============================================================================
bool IsExitApp{ false };
IEngineApp* thisIEngineApp{ nullptr };
//=============================================================================
void ExitApp()
{
	IsExitApp = true;
	// TODO: возможно выходить средствами glfw
}
//=============================================================================
#if defined(_DEBUG)
void messageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param) noexcept
{
	auto const src_str = [source]() {
		switch (source)
		{
		case GL_DEBUG_SOURCE_API: return "API";
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
		case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
		case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
		case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
		case GL_DEBUG_SOURCE_OTHER: return "OTHER";
		}
		return "";
		}();

	auto const type_str = [type]() {
		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR: return "ERROR";
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
		case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
		case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
		case GL_DEBUG_TYPE_MARKER: return "MARKER";
		case GL_DEBUG_TYPE_OTHER: return "OTHER";
		}
		return "";
		}();

	auto const severity_str = [severity]() {
		switch (severity) {
		case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
		case GL_DEBUG_SEVERITY_LOW: return "LOW";
		case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
		case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
		}
		return "";
		}();

	std::string msg = src_str + std::string(", ") + type_str + std::string(", ") + severity_str + std::string(", ") + std::to_string(id) + ": " + message;
	Error(msg);
}
#endif
//=============================================================================
void windowIconifyCallback(GLFWwindow* window, int minimized) noexcept
{

}
//=============================================================================
void windowMaximizeCallback(GLFWwindow* window, int maximized) noexcept
{

}
//=============================================================================
void cursorEnterCallback(GLFWwindow* window, int entered) noexcept
{

}
//=============================================================================
void framebufferSizeCallback([[maybe_unused]] GLFWwindow* window, int width, int height) noexcept
{
	if (width < 0 || height < 0) return;
	width = std::max(width, 1);
	height = std::max(height, 1);

	if (thisIEngineApp)
		thisIEngineApp->windowResize(width, height);
}
//=============================================================================
void keyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods) noexcept
{
	if (key < 0) return;

	ImGui_ImplGlfw_KeyCallback(window, key, scanCode, action, mods);

	if (key >= 0 && key < MaxKeys)
	{
		if (action == GLFW_PRESS)
		{
			thisIEngineApp->m_keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			thisIEngineApp->m_keys[key] = false;
		}
		else if (action == GLFW_REPEAT)
		{
			thisIEngineApp->m_repeatKeys[key] = true;
		}
	}
	//std::string keyName = glfwGetKeyName(key, 0);
}
//=============================================================================
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) noexcept
{
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

	if (button >= 0 && button < MaxMouseButtons)
	{
		if (action == GLFW_PRESS)
		{
			thisIEngineApp->m_mouseButtons[button] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			thisIEngineApp->m_mouseButtons[button] = false;
		}
	}
}
//=============================================================================
void mouseCursorPosCallback([[maybe_unused]] GLFWwindow* window, double xpos, double ypos) noexcept
{
	thisIEngineApp->m_currentMousePositionX = xpos;
	thisIEngineApp->m_currentMousePositionY = ypos;
}
//=============================================================================
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) noexcept
{
	ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
}
//=============================================================================
void charCallback(GLFWwindow* window, unsigned int c) noexcept
{
	ImGui_ImplGlfw_CharCallback(window, c);
}
//=============================================================================
void IEngineApp::Run()
{
	if (create())
	{
		float lastFrame = 0.0f;
		while (!shouldWindowClose())
		{
			float currentFrame = static_cast<float>(glfwGetTime());
			m_deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			m_mouseDeltaX = m_currentMousePositionX - m_lastMouseX;
			m_mouseDeltaY = m_currentMousePositionY - m_lastMouseY;
			m_lastMouseX = m_currentMousePositionX;
			m_lastMouseY = m_currentMousePositionY;

			profiler::BeginFrame();

			{
				SE_SCOPED_SAMPLE("Update");
				OnUpdate(m_deltaTime);
			}
			
			{
				SE_SCOPED_SAMPLE("Render");
				OnRender();
			}

			{
				SE_SCOPED_SAMPLE("ImGui Draw");
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();

				OnImGuiDraw();

				ImGui::Render();
				ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			}

			memset(m_repeatKeys.data(), false, m_repeatKeys.size());
			
			profiler::EndFrame();
			glfwSwapBuffers(m_window);
			glfwPollEvents();
		}
	}
	destroy();
}
//=============================================================================
float IEngineApp::GetAspect() const
{
	return (float)m_width / (float)m_height;
}
//=============================================================================
bool IEngineApp::GetKeyDown(int key)
{
	return m_keys[key];
}
//=============================================================================
bool IEngineApp::GetKeyPressed(int key)
{
	return m_repeatKeys[key];
}
//=============================================================================
bool IEngineApp::GetMouseButton(int button)
{
	return m_mouseButtons[button];
}
//=============================================================================
double IEngineApp::GetTimeInSec() const
{
	return glfwGetTime();
}
//=============================================================================
void IEngineApp::SetCursorPosition(const glm::uvec2& position)
{
	glfwSetCursorPos(m_window, static_cast<double>(position.x), static_cast<double>(position.y));
}
//=============================================================================
void IEngineApp::DrawProfilerInfo()
{
	profiler::Ui();
}
//=============================================================================
bool IEngineApp::create()
{
	auto engineConfig = GetConfig();

	if (!createWindow(engineConfig))
		return false;

	initOpenGL();

	initImGui();

	if (!m_graphics.Create())
		return false;

	profiler::Init();

	thisIEngineApp = this;
	IsExitApp = false;
	return OnCreate();
}
//=============================================================================
bool IEngineApp::createWindow(const EngineConfig& config)
{
	glfwSetErrorCallback([](int e, const char* str) { Fatal("GLTF Context error(" + std::to_string(e) + "): " + str); });

	if (!glfwInit())
	{
		Fatal("Failed to initialize GLFW");
		return false;
	}

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(_DEBUG)
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#else
	glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_TRUE);
#endif

	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_MAXIMIZED, config.window.maximized ? GL_TRUE : GL_FALSE);

	// Disable GlFW auto iconify behaviour
	// Auto Iconify automatically minimizes (iconifies) the window if the window loses focus additionally auto iconify restores the hardware resolution of the monitor if the window that loses focus is a fullscreen window
	glfwWindowHint(GLFW_AUTO_ICONIFY, 0);

	m_window = glfwCreateWindow(config.window.width, config.window.height, config.window.title.data(), nullptr, nullptr);
	if (!m_window)
	{
		Fatal("Failed to create GLFW window");
		return false;
	}

	glfwSetWindowIconifyCallback(m_window, windowIconifyCallback);
	glfwSetWindowMaximizeCallback(m_window, windowMaximizeCallback);
	glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);

	glfwSetKeyCallback(m_window, keyCallback);
	glfwSetCharCallback(m_window, charCallback);
	glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
	glfwSetCursorPosCallback(m_window, mouseCursorPosCallback);
	glfwSetScrollCallback(m_window, mouseScrollCallback);
	glfwSetCursorEnterCallback(m_window, cursorEnterCallback);

	int displayW, displayH;
	glfwGetFramebufferSize(m_window, &displayW, &displayH);
	windowResize(displayW, displayH);

	Info("Created window");

	glfwMakeContextCurrent(m_window);

	if (gladLoadGL(glfwGetProcAddress) == 0)
	{
		Fatal("Failed to initialize OpenGL context!");
		return false;
	}
	glfwSwapInterval(config.render.vsync ? 1 : 0);

	Info("Created OpenGL context");

	return true;
}
//=============================================================================
void IEngineApp::initOpenGL()
{
#if defined(_DEBUG)
	glDebugMessageCallback(messageCallback, nullptr);
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
#endif

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}
//=============================================================================
void IEngineApp::initImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(m_window, false);
	ImGui_ImplOpenGL3_Init("#version 150");
	ImGui::StyleColorsDark();

	GLFWmonitor* primary = glfwGetPrimaryMonitor();

	float xscale, yscale;
	glfwGetMonitorContentScale(primary, &xscale, &yscale);

	ImGuiStyle* style = &ImGui::GetStyle();
	style->ScaleAllSizes(xscale > yscale ? xscale : yscale);

	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = xscale > yscale ? xscale : yscale;
	io.IniFilename = nullptr;
}
//=============================================================================
bool IEngineApp::shouldWindowClose() const
{
	return IsExitApp || glfwWindowShouldClose(m_window);
}
//=============================================================================
void IEngineApp::destroy()
{
	OnDestroy();

	profiler::Close();

	m_graphics.Destroy();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	if (m_window)
	{
		glfwDestroyWindow(m_window);
		m_window = nullptr;
	}
	glfwTerminate();

	thisIEngineApp = nullptr;
}
//=============================================================================
void IEngineApp::windowResize(int width, int height)
{
	m_width = static_cast<uint32_t>(width);
	m_height = static_cast<uint32_t>(height);
	OnResize(m_width, m_height);
}
//=============================================================================