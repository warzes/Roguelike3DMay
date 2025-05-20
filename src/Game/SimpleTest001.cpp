#include "stdafx.h"
#include "SimpleTest001.h"
//=============================================================================
namespace
{
	const char* shaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec3 aVertexPosition;
layout(location = 1) in vec3 aVertexNormal;
layout(location = 2) in vec2 aVertexTexCoords;

out VS_DATA
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} vsOut;

layout(std140, binding = 0) uniform MVPData {
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
};

void main()
{
	vec4 worldSpacePosition = modelMatrix * vec4(aVertexPosition, 1.0);
	gl_Position = projectionMatrix * viewMatrix * worldSpacePosition;

	vsOut.FragPos = worldSpacePosition.xyz;
	vsOut.Normal = transpose(inverse(mat3(modelMatrix))) * aVertexNormal;
	vsOut.TexCoords = aVertexTexCoords;
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

in VS_DATA
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} fsIn;

layout(binding = 0) uniform sampler2D diffuseTexture1;

out vec4 FragColorOut;

void main()
{
	FragColorOut = texture(diffuseTexture1, fsIn.TexCoords);
}
)";

	struct alignas(16) MVPData final
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
	};

	gl4::ShaderProgram program;

	MVPData mvpData;
	GLuint mvpUbo;

	GLuint texture;
	gl4::Buffer vbo;
	gl4::VertexArray vao;

	Camera camera;
}
//=============================================================================
EngineConfig SimpleTest001::GetConfig() const
{
	return {};
}
//=============================================================================
bool SimpleTest001::OnCreate()
{
	program = gl4::CreateShaderProgram(shaderCodeVertex, shaderCodeFragment);

	mvpUbo = gl4::CreateBufferStorage(GL_DYNAMIC_STORAGE_BIT, sizeof(MVPData), nullptr);
	
	texture = gl4::LoadTexture2D("ExampleData/textures/wood.png", false);

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 uv;
	};

	std::vector<gl4::VertexAttribute> attribs = {
		{0, 3, GL_FLOAT, false, offsetof(Vertex, pos)},
		{1, 3, GL_FLOAT, false, offsetof(Vertex, normal)},
		{2, 2, GL_FLOAT, false, offsetof(Vertex, uv)},
	};

	// Quad
	float vertices[]{
		// positions            // normals            // texcoords
		 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
		-10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,

		 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
		-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
		 10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f
	};

	vbo = gl4::CreateBufferStorage(0, sizeof(vertices), vertices);
	vao = gl4::CreateVertexArray(vbo, sizeof(Vertex), attribs);

	camera.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

	glClearColor(0.7f, 0.8f, 0.9f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	return true;
}
//=============================================================================
void SimpleTest001::OnDestroy()
{
}
//=============================================================================
void SimpleTest001::OnUpdate(float deltaTime)
{
	if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(CameraForward, deltaTime);
	if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(CameraBackward, deltaTime);
	if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(CameraLeft, deltaTime);
	if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(CameraRight, deltaTime);

	if (glfwGetMouseButton(GetGLFWWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		SetCursorVisible(false);
		camera.ProcessMouseMovement(-GetMouseDeltaX(), -GetMouseDeltaY());
	}
	else if (glfwGetMouseButton(GetGLFWWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
	{
		glfwSetInputMode(GetGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		SetCursorVisible(true);
	}
}
//=============================================================================
void SimpleTest001::OnRender()
{
	gl4::SetFrameBuffer({ 0 }, GetWidth(), GetHeight(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mvpData.model = glm::mat4(1.0f);
	mvpData.view = camera.GetViewMatrix();
	mvpData.projection = glm::perspective(glm::radians(60.0f), GetAspect(), 0.01f, 1000.0f);

	glNamedBufferSubData(mvpUbo, 0, sizeof(MVPData), &mvpData);

	// вывод квада плоскости
	{
		glUseProgram(program);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, mvpUbo);
		glBindTextureUnit(0, texture);
		glBindVertexArray(vao);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
}
//=============================================================================
void SimpleTest001::OnImGuiDraw()
{
	ImGui::Begin("Simple");

	ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Vendor: %s", (char*)glGetString(GL_VENDOR));
	ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Version: %s", (char*)glGetString(GL_VERSION));
	ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Renderer: %s", (char*)glGetString(GL_RENDERER));
	ImGui::Separator();

	ImGui::Text("Framerate: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();
}
//=============================================================================
void SimpleTest001::OnResize(uint16_t width, uint16_t height)
{
}
//=============================================================================