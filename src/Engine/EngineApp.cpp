#include "stdafx.h"
#include "EngineApp.h"
#include "Log.h"
#include "Profiler.h"
#include "OpenGL4Advance.h"
//=============================================================================
// Use the high-performance GPU (if available) on Windows laptops
// https://docs.nvidia.com/gameworks/content/technologies/desktop/optimus.htm
// https://gpuopen.com/learn/amdpowerxpressrequesthighperformance/
#if defined(_WIN32)
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = 1;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
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
void openGLErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, [[maybe_unused]] GLsizei length, const GLchar* message, [[maybe_unused]] const void* user_param) noexcept
{
	// Ignore certain verbose info messages (particularly ones on Nvidia).
	if (id == 131169 ||
		id == 131185 || // NV: Buffer will use video memory
		id == 131218 ||
		id == 131204 || // Texture cannot be used for texture mapping
		id == 131222 ||
		id == 131154 || // NV: pixel transfer is synchronized with 3D rendering
		id == 0         // gl{Push, Pop}DebugGroup
		)
		return;

	const auto sourceStr = [source]() {
		switch (source)
		{
		case GL_DEBUG_SOURCE_API:             return "Source: API";
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "Source: Window Manager";
		case GL_DEBUG_SOURCE_SHADER_COMPILER: return "Source: Shader Compiler";
		case GL_DEBUG_SOURCE_THIRD_PARTY:     return "Source: Third Party";
		case GL_DEBUG_SOURCE_APPLICATION:     return "Source: Application";
		case GL_DEBUG_SOURCE_OTHER:           return "Source: Other";
		}
		return "";
		}();

	const auto typeStr = [type]() {
		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR:               return "Type: Error";
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Type: Deprecated Behavior";
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "Type: Undefined Behavior";
		case GL_DEBUG_TYPE_PORTABILITY:         return "Type: Portability";
		case GL_DEBUG_TYPE_PERFORMANCE:         return "Type: Performance";
		case GL_DEBUG_TYPE_MARKER:              return "Type: Marker";
		case GL_DEBUG_TYPE_PUSH_GROUP:          return "Type: Push Group";
		case GL_DEBUG_TYPE_POP_GROUP:           return "Type: Pop Group";
		case GL_DEBUG_TYPE_OTHER:               return "Type: Other";
		}
		return "";
		}();

	const auto severityStr = [severity]() {
		switch (severity) {
		case GL_DEBUG_SEVERITY_NOTIFICATION: return "Severity: notification";
		case GL_DEBUG_SEVERITY_LOW:          return "Severity: low";
		case GL_DEBUG_SEVERITY_MEDIUM:       return "Severity: medium";
		case GL_DEBUG_SEVERITY_HIGH:         return "Severity: high";
		}
		return "";
		}();

	const std::string msg = "OpenGL Debug message(id=" + std::to_string(id) + "):\n"
		+ sourceStr + '\n'
		+ typeStr + '\n'
		+ severityStr + '\n'
		+ "Message: " + std::string(message) + '\n';
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
		float lastTime = 0.0f;
		while (!shouldWindowClose())
		{
			// calc deltatime
			float currentTime = static_cast<float>(glfwGetTime());
			m_deltaTime = currentTime - lastTime;
			lastTime = currentTime;

			m_mouseDeltaX = m_currentMousePositionX - m_mouseLastX;
			m_mouseDeltaY = m_currentMousePositionY - m_mouseLastY;
			m_mouseLastX = m_currentMousePositionX;
			m_mouseLastY = m_currentMousePositionY;

			profiler::BeginFrame();

			{
				SE_SCOPED_SAMPLE("Update");
				OnUpdate(m_deltaTime);
			}
			
			{
				SE_SCOPED_SAMPLE("Frame");
				// Start a new ImGUi frame
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();

				OnRender();
				OnImGuiDraw();

				// Updates ImGui
				ImGui::Render();
				auto* drawData = ImGui::GetDrawData();
				if (drawData->CmdListsCount > 0)
				{
					// A frame marker is inserted to distinguish ImGui rendering from the application's in a debugger.
					auto marker = gl4A::ScopedDebugMarker("Draw ImGui");
					glDisable(GL_FRAMEBUFFER_SRGB);
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
					ImGui_ImplOpenGL3_RenderDrawData(drawData);
					glEnable(GL_FRAMEBUFFER_SRGB);
				}
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
	m_mouseLastX = position.x;
	m_mouseLastY = position.y;
}
//=============================================================================
void IEngineApp::DrawProfilerInfo()
{
	profiler::Ui();
}
//=============================================================================
void IEngineApp::SetCursorVisible(bool visible)
{
	if (m_cursorVisible != visible)
	{
		m_cursorVisible = visible;
		glfwSetInputMode(m_window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
		SetCursorPosition({ m_width / 2, m_height / 2 });
	}
}
//=============================================================================
bool IEngineApp::create()
{
	auto engineConfig = GetConfig();

	void ClearOpenGLState();
	ClearOpenGLState();

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
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_MAXIMIZED, config.window.maximized ? GL_TRUE : GL_FALSE);
	glfwWindowHint(GLFW_DECORATED, config.window.decorate ? GL_TRUE : GL_FALSE);

	// Disable GlFW auto iconify behaviour
	// Auto Iconify automatically minimizes (iconifies) the window if the window loses focus additionally auto iconify restores the hardware resolution of the monitor if the window that loses focus is a fullscreen window
	glfwWindowHint(GLFW_AUTO_ICONIFY, GL_FALSE);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	if (!monitor)
	{
		Fatal("No Monitor detected");
		return false;
	}
	const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);

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
	m_width = static_cast<uint16_t>(displayW);
	m_height = static_cast<uint16_t>(displayH);

	int monitorLeft, monitorTop;
	glfwGetMonitorPos(monitor, &monitorLeft, &monitorTop);

	glfwSetWindowPos(m_window,
		videoMode->width / 2 - m_width / 2 + monitorLeft,
		videoMode->height / 2 - m_height / 2 + monitorTop);

	double xpos, ypos;
	glfwGetCursorPos(m_window, &xpos, &ypos);
	m_mouseLastX = xpos;
	m_mouseLastY = ypos;

	glfwMakeContextCurrent(m_window);

	const int openGLVersion = gladLoadGL(glfwGetProcAddress);
	if (openGLVersion < GLAD_MAKE_VERSION(4, 6))
	{
		Fatal("Failed to initialize OpenGL context!");
		return false;
	}

	glfwSwapInterval(config.render.vsync ? 1 : 0);

	return true;
}
//=============================================================================
void IEngineApp::initOpenGL()
{
#if defined(_DEBUG)
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(openGLErrorCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#endif

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glEnable(GL_FRAMEBUFFER_SRGB);
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

	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;// TODO: возможно есть другой способ как имгуи не давать показывать скрытый курсор
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
uint16_t GetWindowWidth()
{
	return thisIEngineApp->GetWidth();
}
//=============================================================================
uint16_t GetWindowHeight()
{
	return thisIEngineApp->GetHeight();
}
//=============================================================================
float GetWindowAspect()
{
	return thisIEngineApp->GetAspect();
}
//=============================================================================
bool GetKeyDown(int key)
{
	return thisIEngineApp->GetKeyDown(key);
}
//=============================================================================
bool GetMouseButton(int button)
{
	return thisIEngineApp->GetMouseButton(button);
}
//=============================================================================
int GetMousePositionX()
{
	return thisIEngineApp->GetMousePositionX();
}
//=============================================================================
int GetMousePositionY()
{
	return thisIEngineApp->GetMousePositionY();
}
//=============================================================================