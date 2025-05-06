#include "stdafx.h"
#include "GameApp.h"
//=============================================================================
namespace
{
	const char* shaderCodeVertex = R"(
#version 460 core

layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aColor;
layout (location=2) in vec2 aUV;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Proj;


out vec3 Color;
out vec2 UV;

void main()
{
	gl_Position = Proj * View * Model * vec4(aPos, 1.0f);
	Color = aColor;
	UV = aUV;
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

layout (location=0) in vec3 Color;
layout (location=1) in vec2 UV;

layout (location=0) out vec4 oFragColor;

layout(binding = 0) uniform sampler2D tex0;

void main()
{
	oFragColor = texture(tex0, UV);
};
)";

	const char* modelShaderCodeVertex = R"(
#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT 
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vs_out.FragPos = mat3(model) * aPos;
	vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
	vs_out.TexCoords = aTexCoords;
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

	const char* modelShaderCodeFragment = R"(
#version 460 core

out vec4 FragColor;

in VS_OUT 
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} fs_in;

layout(binding = 0) uniform sampler2D texture_diffuse1; // Texture from glTF model

void main()
{
	vec3 color = texture(texture_diffuse1, fs_in.TexCoords).rgb;
	FragColor = vec4(color, 1.0);
}
)";

	GLuint program;
	int ModelLoc;
	int ViewLoc;
	int ProjLoc;

	GLuint modelProgram;
	int modelModelLoc;
	int modelViewLoc;
	int modelProjLoc;

	GLuint texture;
	GLuint vbo;
	GLuint ibo;
	GLuint vao;

	Camera camera;

	Model* model;
}
//=============================================================================
EngineConfig GameApp::GetConfig() const
{
	return {};
}
//=============================================================================
bool GameApp::OnCreate()
{
	program = gl4::CreateShaderProgram(shaderCodeVertex, shaderCodeFragment);
	ModelLoc = gl4::GetUniformLocation(program, "Model");
	ViewLoc = gl4::GetUniformLocation(program, "View");
	ProjLoc = gl4::GetUniformLocation(program, "Proj");

	modelProgram = gl4::CreateShaderProgram(modelShaderCodeVertex, modelShaderCodeFragment);
	modelModelLoc = gl4::GetUniformLocation(modelProgram, "model");
	modelViewLoc = gl4::GetUniformLocation(modelProgram, "view");
	modelProjLoc = gl4::GetUniformLocation(modelProgram, "projection");

	texture = gl4::LoadTexture2D("data/textures/colorful.png", true);

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 uv;
	};

	std::vector<gl4::VertexAttribute> attribs = {
		{0, 3, GL_FLOAT, false, offsetof(Vertex, pos)},
		{1, 3, GL_FLOAT, false, offsetof(Vertex, color)},
		{2, 2, GL_FLOAT, false, offsetof(Vertex, uv)},
	};

	Vertex vertices[] =
	{
		{{-0.5f, -0.5f, 0.0f},	{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{-0.5f,  0.5f, 0.0f},	{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
		{{ 0.5f,  0.5f, 0.0f},	{0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{ 0.5f, -0.5f, 0.0f},	{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
	};

	GLuint indices[] =
	{
		0, 2, 1,
		0, 3, 2
	};

	vbo = gl4::CreateBuffer(0, sizeof(vertices), vertices);
	ibo = gl4::CreateBuffer(0, sizeof(indices), indices);
	vao = gl4::CreateVertexArray(vbo, ibo, sizeof(Vertex), attribs);

	camera.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

	model = new Model("data/mesh/Zaku/scene.gltf");

	glClearColor(0.7f, 0.8f, 0.9f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	return true;
}
//=============================================================================
void GameApp::OnDestroy()
{
	glDeleteTextures(1, &texture);
	glDeleteProgram(program);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
	glDeleteVertexArrays(1, &vao);
}
//=============================================================================
void GameApp::OnUpdate(float deltaTime)
{
	if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(CameraForward, deltaTime);
	if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(CameraBackward, deltaTime);
	if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(CameraLeft, deltaTime);
	if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(CameraRight, deltaTime);

	if (glfwGetMouseButton(GetGLFWWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		SetCursorVisible(false);
		camera.ProcessMouseMovement(GetMouseDeltaX(), GetMouseDeltaY());
	}
	else if (glfwGetMouseButton(GetGLFWWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		glfwSetInputMode(GetGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		SetCursorVisible(true);
	}
}
//=============================================================================
void GameApp::OnRender()
{
	glViewport(0, 0, GetWidth(), GetHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);

	glm::mat4 matmodel = glm::mat4(1.0f);
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 proj = glm::perspective(glm::radians(60.0f), GetAspect(), 0.01f, 1000.0f);

	gl4::SetUniform(ModelLoc, matmodel);
	gl4::SetUniform(ViewLoc, view);
	gl4::SetUniform(ProjLoc, proj);

	glBindTextureUnit(0, texture);
	glBindVertexArray(vao);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glUseProgram(modelProgram);

	gl4::SetUniform(modelModelLoc, matmodel);
	gl4::SetUniform(modelViewLoc, view);
	gl4::SetUniform(modelProjLoc, proj);
	model->Draw(modelProgram);
}
//=============================================================================
void GameApp::OnImGuiDraw()
{
	DrawProfilerInfo();
}
//=============================================================================
void GameApp::OnResize(uint16_t width, uint16_t height)
{

}
//=============================================================================