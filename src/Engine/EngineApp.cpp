#include "stdafx.h"
#include "EngineApp.h"
#include "Log.h"
//=============================================================================
#if defined(_WIN32)
extern "C" {
	_declspec(dllexport) uint32_t NvOptimusEnablement = 1;
	_declspec(dllexport) uint32_t AmdPowerXpressRequestHighPerformance = 1;
}
#endif
//=============================================================================
bool IsExitApp{ false };
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
void IEngineApp::Run()
{
	if (create())
	{
		while (!shouldWindowClose())
		{
			glfwSwapBuffers(m_window);
			glfwPollEvents();
		}
	}
	destroy();
}
//=============================================================================
bool IEngineApp::create()
{
	auto engineConfig = GetConfig();

	glfwSetErrorCallback([](int /*error*/, const char* description)
		{
			Fatal(description);
		}
	);

	if (!glfwInit())
	{
		Fatal("Failed to initialize glfw");
		return false;
	}

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

	m_window = glfwCreateWindow(engineConfig.window.width, engineConfig.window.height, engineConfig.window.title.data(), nullptr, nullptr);
	if (!m_window)
	{
		Fatal("Failed to create GLFW window");
		return false;
	}

	Info("Created window");

	glfwMakeContextCurrent(m_window);

	if (gladLoadGL(glfwGetProcAddress) == 0)
	{
		Fatal("Failed to load GLfunc");
		return false;
	}
	glfwSwapInterval(1);

#if defined(_DEBUG)
	glDebugMessageCallback(messageCallback, nullptr);
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
#endif

	Info("Created OpenGL context");

	IsExitApp = false;
	return OnCreate();
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

	if (m_window)
	{
		glfwDestroyWindow(m_window);
		m_window = nullptr;
	}
	glfwTerminate();
}
//=============================================================================