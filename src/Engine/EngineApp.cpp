#include "stdafx.h"
#include "EngineApp.h"
#include "Log.h"
#include "Profiler.h"
#include "OpenGL4DebugMarker.h"
#include "OpenGL4Context.h"
#include "TextureManager.h"
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
IEngineApp* thisIEngineApp{ nullptr };
//=============================================================================
void ExitApp()
{
	if (thisIEngineApp)
		thisIEngineApp->Exit();
}
//=============================================================================
uint16_t GetWindowWidth()
{
	return thisIEngineApp->GetWindowWidth();
}
//=============================================================================
uint16_t GetWindowHeight()
{
	return thisIEngineApp->GetWindowHeight();
}
//=============================================================================
float GetWindowAspect()
{
	return thisIEngineApp->GetWindowAspect();
}
//=============================================================================
//bool GetKeyDown(int key)
//{
//	return thisIEngineApp->GetKeyDown(key);
//}
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
#if defined(_DEBUG)
namespace
{
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
}
#endif
//=============================================================================
void windowPosCallback(
	[[maybe_unused]] GLFWwindow* window,
	int xpos, int ypos) noexcept
{
	if (thisIEngineApp)
	{
		thisIEngineApp->m_windowPosition.x = xpos;
		thisIEngineApp->m_windowPosition.y = ypos;
	}
}
//=============================================================================
void windowFocusCallback([[maybe_unused]] GLFWwindow* window,
	[[maybe_unused]] int focused) noexcept
{

}
//=============================================================================
void windowIconifyCallback(
	[[maybe_unused]] GLFWwindow* window,
	[[maybe_unused]] int minimized) noexcept
{

}
//=============================================================================
void windowMaximizeCallback(
	[[maybe_unused]] GLFWwindow* window,
	[[maybe_unused]] int maximized) noexcept
{

}
//=============================================================================
void windowCloseCallback([[maybe_unused]] GLFWwindow* window) noexcept
{

}
//=============================================================================
void framebufferSizeCallback(
	[[maybe_unused]] GLFWwindow* window,
	int width, int height) noexcept
{
	if (width < 0 || height < 0) return;
	width = std::max(width, 1);
	height = std::max(height, 1);

	if (thisIEngineApp)
		thisIEngineApp->windowResize(width, height);
}
//=============================================================================
void cursorEnterCallback(
	[[maybe_unused]] GLFWwindow* window,
	[[maybe_unused]] int entered) noexcept
{

}
//=============================================================================
void keyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods) noexcept
{
	ImGui_ImplGlfw_KeyCallback(window, key, scanCode, action, mods);
	thisIEngineApp->keypress(key, scanCode, action, mods);
}
//=============================================================================
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) noexcept
{
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
	thisIEngineApp->mouseButton(button, action, mods);
}
//=============================================================================
void mouseCursorPosCallback([[maybe_unused]] GLFWwindow* window, double xpos, double ypos) noexcept
{
	thisIEngineApp->mousePos(xpos, ypos);
}
//=============================================================================
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) noexcept
{
	ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
	thisIEngineApp->mouseScroll(xoffset, yoffset);
}
//=============================================================================
void charCallback(GLFWwindow* window, unsigned int c) noexcept
{
	ImGui_ImplGlfw_CharCallback(window, c);
}
//=============================================================================
void IEngineApp::Run()
{
	if (init())
	{
		double lastTime = glfwGetTime();
		while (!shouldWindowClose())
		{
			profiler::BeginFrame();

			// update
			{
				SE_SCOPED_SAMPLE("Update");

				// calc deltatime
				const double currentTime = glfwGetTime();
				m_deltaTime = static_cast<float>(currentTime - lastTime);
				lastTime = currentTime;
				// calc fps
				fpsTick(m_deltaTime);

				m_mouseDeltaX = m_currentMousePositionX - m_mouseLastX;
				m_mouseDeltaY = m_currentMousePositionY - m_mouseLastY;
				m_mouseLastX = m_currentMousePositionX;
				m_mouseLastY = m_currentMousePositionY;

				OnUpdate(m_deltaTime);
			}

			// render
			if (m_canRender)
			{
				{
					SE_SCOPED_SAMPLE("Render");
					OnRender();
				}

				{
					SE_SCOPED_SAMPLE("ImGui Frame");
					// Start a new ImGUi frame
					ImGui_ImplOpenGL3_NewFrame();
					ImGui_ImplGlfw_NewFrame();
					ImGui::NewFrame();

					OnImGuiDraw();

					// Updates ImGui
					ImGui::Render();
					auto* drawData = ImGui::GetDrawData();
					if (drawData->CmdListsCount > 0)
					{
						// A frame marker is inserted to distinguish ImGui rendering from the application's in a debugger.
						auto marker = gl::ScopedDebugMarker("Draw ImGui");
						glDisable(GL_FRAMEBUFFER_SRGB);
						glBindFramebuffer(GL_FRAMEBUFFER, 0);
						ImGui_ImplOpenGL3_RenderDrawData(drawData);
						glEnable(GL_FRAMEBUFFER_SRGB);
					}
				}
			}

			if (!m_cursorVisible)
			{
				SetCursorPosition({ m_width / 2, m_height / 2 });
			}
			glfwSwapBuffers(m_window);

			Input::Update();

			profiler::EndFrame();
		}
	}
	close();
}
//=============================================================================
void IEngineApp::Exit()
{
	glfwSetWindowShouldClose(m_window, GLFW_TRUE);
}
//=============================================================================
//bool IEngineApp::GetKeyDown(int key)
//{
//	if (key < 0 || key >= (int)MaxKeys) return false;
//	return m_keys[static_cast<size_t>(key)];
//}
////=============================================================================
//bool IEngineApp::GetKeyPressed(int key)
//{
//	if (key < 0 || key >= (int)MaxKeys) return false;
//	return m_repeatKeys[static_cast<size_t>(key)];
//}
//=============================================================================
bool IEngineApp::GetMouseButton(int button)
{
	if (button < 0 || button >= (int)MaxMouseButtons) return false;
	return m_mouseButtons[static_cast<size_t>(button)];
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
	m_mouseLastX = m_currentMousePositionX = position.x;
	m_mouseLastY = m_currentMousePositionY = position.y;
	m_mouseDeltaX = m_mouseDeltaY = 0.0f;
}
//=============================================================================
void IEngineApp::DrawProfilerInfo()
{
	profiler::Ui();
}
//=============================================================================
void IEngineApp::DrawFPS()
{
	if (const ImGuiViewport* v = ImGui::GetMainViewport()) {
		ImGui::SetNextWindowPos({ v->WorkPos.x + v->WorkSize.x - 15.0f, v->WorkPos.y + 15.0f }, ImGuiCond_Always, { 1.0f, 0.0f });
	}
	ImGui::SetNextWindowBgAlpha(0.30f);
	ImGui::SetNextWindowSize(ImVec2(ImGui::CalcTextSize("FPS : _______").x, 0));
	if (ImGui::Begin("##FPS",
		nullptr,
		ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove)) {
		ImGui::Text("FPS : %i", (int)m_currentFPS);
		ImGui::Text("Ms  : %.1f", m_currentFPS > 0 ? 1000.0 / m_currentFPS : 0);
	}
	ImGui::End();
}
//=============================================================================
void IEngineApp::SetCursorVisible(bool visible)
{
	if (m_cursorVisible != visible)
	{
		m_cursorVisible = visible;
		glfwSetInputMode(m_window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
	}
}
//=============================================================================
bool IEngineApp::init()
{
	auto engineConfig = GetCreateInfo();

	if (!initWindow(engineConfig))
		return false;
	initOpenGL();
	initImGui();

	if (!TextureManager::Init())
		return false;

	profiler::Init();
	thisIEngineApp = this;
	return OnInit();
}
//=============================================================================
bool IEngineApp::initWindow(const EngineCreateInfo& config)
{
	glfwSetErrorCallback([](int e, const char* str) noexcept { Fatal("GLTF Context error(" + std::to_string(e) + "): " + str); });

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
	glfwWindowHint(GLFW_RESIZABLE, config.window.resizable ? GLFW_TRUE : GLFW_FALSE);
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

	// Window callbacks
	glfwSetWindowPosCallback(m_window, windowPosCallback);
	glfwSetWindowFocusCallback(m_window, windowFocusCallback);
	glfwSetWindowIconifyCallback(m_window, windowIconifyCallback);
	glfwSetWindowMaximizeCallback(m_window, windowMaximizeCallback);
	glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
	glfwSetWindowCloseCallback(m_window, windowCloseCallback);

	// Key callbacks 
	glfwSetKeyCallback(m_window, keyCallback);
	glfwSetCharCallback(m_window, charCallback);

	// Mouse callbacks 
	glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
	glfwSetCursorPosCallback(m_window, mouseCursorPosCallback);
	glfwSetScrollCallback(m_window, mouseScrollCallback);
	glfwSetCursorEnterCallback(m_window, cursorEnterCallback);

	Input::Init(m_window);

	int displayW, displayH;
	glfwGetFramebufferSize(m_window, &displayW, &displayH);
	m_width = static_cast<uint16_t>(displayW);
	m_height = static_cast<uint16_t>(displayH);
	m_windowAspect = static_cast<float>(m_width) / static_cast<float>(m_height);

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

	glEnable(GL_FRAMEBUFFER_SRGB);

	glDisable(GL_DITHER);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	gl::gContext.Init();
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
	return glfwWindowShouldClose(m_window) == GLFW_TRUE;
}
//=============================================================================
void IEngineApp::close()
{
	OnClose();

	TextureManager::Close();

	gl::gContext.Close();

	profiler::Close();

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
	if (width == m_width && height == m_height) return;
	m_width = static_cast<uint16_t>(width);
	m_height = static_cast<uint16_t>(height);
	m_windowAspect = static_cast<float>(width) / static_cast<float>(height);
	OnResize(m_width, m_height);
}
//=============================================================================
void IEngineApp::fpsTick(float deltaSeconds, bool frameRendered)
{
	if (frameRendered)
		m_numFrames++;

	m_accumulatedTime += deltaSeconds;

	if (m_accumulatedTime > m_avgInterval)
	{
		m_currentFPS = static_cast<float>(m_numFrames / m_accumulatedTime);
		m_numFrames = 0;
		m_accumulatedTime = 0;
	}
}
//=============================================================================
void IEngineApp::keypress(int key, int scanCode, int action, int mods)
{
	//std::string keyName = glfwGetKeyName(key, 0);	
	Input::keypress(key, action);
	OnKey(key, scanCode, action, mods);
}
//=============================================================================
void IEngineApp::mousePos(double xpos, double ypos)
{
	m_currentMousePositionX = xpos;
	m_currentMousePositionY = ypos;
	Input::mousePos(xpos, ypos);
	OnMousePos(xpos, ypos);
}
//=============================================================================
void IEngineApp::mouseScroll(double xoffset, double yoffset)
{
	Input::mouseScroll(xoffset, yoffset);
	OnScroll(xoffset, yoffset);
}
//=============================================================================
void IEngineApp::mouseButton(int button, int action, int mods)
{
	if (button >= 0 && button < (int)MaxMouseButtons)
	{
		if (action == GLFW_PRESS)
		{
			m_mouseButtons[static_cast<size_t>(button)] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			m_mouseButtons[static_cast<size_t>(button)] = false;
		}
	}
	Input::mouseButton(button, action);
	OnMouseButton(button, action, mods);
}
//=============================================================================