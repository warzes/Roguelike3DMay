#if 0

#include <Windows.h>
#include <gl/GL.h>
#pragma comment(lib, "opengl32.lib")

#if defined(_WIN32)
extern "C"
{
	//__declspec(dllexport) unsigned long NvOptimusEnablement = 1;
	//__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#define GL_PROC(ret, name, ...) using PFN_##name = ret(__stdcall*)(__VA_ARGS__); \
                                inline PFN_##name name = nullptr;
#include "glProc.inl"
#undef GL_PROC

bool running{ true };
int windowWidth{ 800 };
int windowHeight{ 600 };

GLuint VBO{ 0 };
GLuint VAO{ 0 };
GLuint program{ 0 };

GLuint CompileShader(GLenum type, const char* source)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);
	return shader;
}

GLuint CreateProgram(const char* vertexSource, const char* fragmentSource)
{
	GLuint vs = CompileShader(GL_VERTEX_SHADER, vertexSource);
	GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

	GLuint program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glDeleteShader(vs);
	glDeleteShader(fs);
	return program;
}

void Init()
{
	const char* vertex_shader_src = R"(
#version 460 core
layout(location = 0) in vec3 aPos;
void main() {
	gl_Position = vec4(aPos, 1.0);
}
)";

	const char* fragment_shader_src = R"(
#version 460 core
out vec4 FragColor;
void main() {
	FragColor = vec4(0.7, 0.4, 0.5, 1.0);
}
)";

	program = CreateProgram(vertex_shader_src, fragment_shader_src);

	const float vertices[] = { -0.5f, -0.5f, 0.0f,     0.5f, -0.5f, 0.0f,     0.0f,  0.5f, 0.0f };
	glCreateBuffers(1, &VBO);
	glNamedBufferStorage(VBO, sizeof(vertices), vertices, 0);

	glCreateVertexArrays(1, &VAO);
	glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(VAO, 0, 0);
	glEnableVertexArrayAttrib(VAO, 0);
}

void Frame()
{
	glClearColor(1.0, 0.8, 0.4, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, windowWidth, windowHeight);

	glUseProgram(program);
	glBindVertexArray(VAO);
	glVertexArrayVertexBuffer(VAO, 0, VBO, 0, 3 * sizeof(float));
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Close()
{
	glDeleteProgram(program);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		running = false;
		PostQuitMessage(0);
		return 0;
	case WM_SIZE:
		windowWidth = LOWORD(lParam);
		windowHeight = HIWORD(lParam);
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void LoadGLFunc()
{
#define GL_PROC(ret, name, ...) name = (PFN_##name)wglGetProcAddress(#name); 
#include "glProc.inl"
#undef GL_PROC
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	constexpr const auto WndClassName{ L"OpenGLWindowClass" };
	constexpr const auto Title{ L"Minimal OpenGL WinAPI" };
	WNDCLASS wc      = { 0 };
	wc.lpfnWndProc   = WndProc;
	wc.hInstance     = hInstance;
	wc.lpszClassName = WndClassName;
	wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	RegisterClass(&wc);
	HWND window = CreateWindowEx(0, WndClassName, Title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight, nullptr, nullptr, hInstance, nullptr);
	ShowWindow(window, nCmdShow);
	UpdateWindow(window);

	HDC hdc = GetDC(window);

	PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		24, 8, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
	};

	int pixelFormat = ChoosePixelFormat(hdc, &pfd);
	SetPixelFormat(hdc, pixelFormat, &pfd);
	HGLRC tempContext = wglCreateContext(hdc);
	wglMakeCurrent(hdc, tempContext);

	using PFN_wglCreateContextAttribs = HGLRC(WINAPI*)(HDC, HGLRC, const int*);
	auto wglCreateContextAttribsARB = (PFN_wglCreateContextAttribs)wglGetProcAddress("wglCreateContextAttribsARB");

	wglMakeCurrent(nullptr, nullptr);
	wglDeleteContext(tempContext);

	const int attribs[] = {
		0x2091 /*WGL_CONTEXT_MAJOR_VERSION_ARB*/, 4,
		0x2092 /*WGL_CONTEXT_MINOR_VERSION_ARB*/, 6,
		0x9126 /*WGL_CONTEXT_PROFILE_MASK_ARB*/,  0x00000001 /*WGL_CONTEXT_CORE_PROFILE_BIT_ARB*/,
#if defined(_DEBUG)
		0x2094 /*WGL_CONTEXT_FLAGS_ARB*/,         0x0001/*WGL_CONTEXT_DEBUG_BIT_ARB*/,
#else
		0x2094 /*WGL_CONTEXT_FLAGS_ARB*/,         0,
#endif // _DEBUG
		0
	};

	HGLRC hrc = wglCreateContextAttribsARB(hdc, nullptr, attribs);
	wglMakeCurrent(hdc, hrc);

	LoadGLFunc();
	
	Init();

	while (running)
	{
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		Frame();
		SwapBuffers(hdc);
	}

	Close();

	wglMakeCurrent(nullptr, nullptr);
	wglDeleteContext(hrc);
	ReleaseDC(window, hdc);
	DestroyWindow(window);
}

#endif