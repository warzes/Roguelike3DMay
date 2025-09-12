#include "stdafx.h"
#include "GameApp.h"
//=============================================================================
#if defined(_MSC_VER)
#	pragma comment( lib, "3rdparty.lib" )
#	pragma comment( lib, "Engine.lib" )
#endif
//=============================================================================
//https://github.com/paul-akl/Praxis3D
//https://github.com/paul-akl/Paradime3D
//https://www.youtube.com/@ShaneCodesSpaghetti/videos
//https://www.youtube.com/@blakedarrow/videos
//https://www.youtube.com/watch?v=PaJpxk4XzK0
//https://www.youtube.com/@ikoukourakis/videos
//https://www.youtube.com/@funitinker/videos
//D:\_project2025\tutorials\OpenGL\Height Map Part6

int main(
	[[maybe_unused]] int   argc,
	[[maybe_unused]] char* argv[])
{
	/*
	https://www.youtube.com/shorts/0hdta4b8hcA - идея
	наоборот качественный рендер
	*/
	GameApp game;
	game.Run();
	return 0;
	
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(1600, 900, "sRGB Example", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	gladLoadGL(glfwGetProcAddress);


	const char* vertex_shader_src = R"(
#version 460 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
gl_Position = vec4(aPos, 0.0, 1.0);
TexCoord = aTexCoord;
}
)";

	const char* fragment_shader_src = R"(
#version 460 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;

void main() {
FragColor = texture(uTexture, TexCoord);
}
)";

	// === Шейдеры ===
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_src, nullptr);
	glCompileShader(vertex_shader);

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_src, nullptr);
	glCompileShader(fragment_shader);

	GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	const float vertices[] = {
	1.0f,  1.0f,   1.0f, 1.0f,
	1.0f, -1.0f,   1.0f, 0.0f,
	-1.0f, -1.0f,   0.0f, 0.0f,
	-1.0f,  1.0f,   0.0f, 1.0f
	};
	const unsigned int indices[] = {
	0, 1, 3,
	1, 2, 3
	};

	// === VAO, VBO, EBO через DSA ===
	GLuint vao, vbo, ebo;
	glCreateVertexArrays(1, &vao);
	glCreateBuffers(1, &vbo);
	glCreateBuffers(1, &ebo);
	glNamedBufferData(vbo, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glNamedBufferData(ebo, sizeof(indices), indices, GL_STATIC_DRAW);

	// Атрибуты: позиция (vec2), текстура (vec2)
	glEnableVertexArrayAttrib(vao, 0);
	glEnableVertexArrayAttrib(vao, 1);
	glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float));
	glVertexArrayAttribBinding(vao, 0, 0);
	glVertexArrayAttribBinding(vao, 1, 0);
	glVertexArrayVertexBuffer(vao, 0, vbo, 0, 4 * sizeof(float));
	glVertexArrayElementBuffer(vao, ebo);
	

	// === Глобально включаем sRGB коррекцию6 ===
	glEnable(GL_FRAMEBUFFER_SRGB);

	// === Загрузка текстуры ===
	stbi_set_flip_vertically_on_load(true);
	int width, height, channels;
	auto data = stbi_loadf("CoreData/textures/grey.png", &width, &height, &channels, STBI_rgb_alpha);

	GLuint texture;
	glCreateTextures(GL_TEXTURE_2D, 1, &texture);
	glTextureStorage2D(texture, 1, GL_SRGB8_ALPHA8, width, height); // sRGB формат!
	glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);

	stbi_image_free(data);
	
	// === Фреймбуфер (рендер в текстуру sRGB) ===
	GLuint fbo, fbo_texture;
	glCreateFramebuffers(1, &fbo);

	// Текстура для FBO — sRGB
	glCreateTextures(GL_TEXTURE_2D, 1, &fbo_texture);
	glTextureStorage2D(fbo_texture, 1, GL_SRGB8_ALPHA8, 1600, 900);
	glTextureParameteri(fbo_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(fbo_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Привязываем текстуру к FBO как цветовой буфер
	glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, fbo_texture, 0);

	// === Основной цикл ===
	while (!glfwWindowShouldClose(window))
	{
		// === Этап 1: рендер в FBO (sRGB текстуру) ===
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glViewport(0, 0, 1600, 900);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(program);
		glBindTextureUnit(0, texture); // Привязка текстуры к юниту 0
		glUniform1i(glGetUniformLocation(program, "uTexture"), 0);
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// === Этап 2: блитинг из FBO на экран ===
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // экран
		glBlitNamedFramebuffer(
			fbo, 0,                    // src, dst
			0, 0, 1600, 900,            // src rect
			0, 0, 1600, 900,            // dst rect
			GL_COLOR_BUFFER_BIT,       // буфер
			GL_LINEAR                  // фильтрация
		);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Очистка
	glDeleteTextures(1, &texture);
	glDeleteTextures(1, &fbo_texture);
	glDeleteFramebuffers(1, &fbo);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteProgram(program);

	glfwDestroyWindow(window);
	glfwTerminate();

}
//=============================================================================