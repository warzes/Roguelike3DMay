#if 0
#include "glad/gl.h"
#include "glfw/glfw3.h"

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

int main(
	[[maybe_unused]] int   argc,
	[[maybe_unused]] char* argv[])
{
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Minimal OpenGL", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	gladLoadGL(glfwGetProcAddress);

	Init();

	while (!glfwWindowShouldClose(window))
	{
		Frame();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	Close();
	glfwTerminate();
}

#endif