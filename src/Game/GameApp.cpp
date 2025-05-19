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

	const char* cubeShaderCodVertex = R"(
# version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
)";
	const char* cubeShaderCodeFragment = R"(
#version 460 core

out vec4 FragColor;

in vec2 TexCoord;

// texture sampler
layout(binding = 0) uniform sampler2D texture1;

void main()
{
	FragColor = texture(texture1, TexCoord);
}
)";



	gl4::ShaderProgram program;
	int ModelLoc;
	int ViewLoc;
	int ProjLoc;

	gl4::ShaderProgram modelProgram;
	int modelModelLoc;
	int modelViewLoc;
	int modelProjLoc;

	gl4::ShaderProgram cubeProgram;
	int cubeModelLoc;
	int cubeViewLoc;
	int cubeProjLoc;

	GLuint texture;
	GLuint vbo;
	GLuint ibo;
	GLuint vao;

	Camera camera;

	Model* model;

	// World space positions of our cubes
	const glm::vec3 cubePositions[]{
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -5.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -2.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -0.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};
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

	cubeProgram = gl4::CreateShaderProgram(cubeShaderCodVertex, cubeShaderCodeFragment);
	cubeModelLoc = gl4::GetUniformLocation(cubeProgram, "model");
	cubeViewLoc = gl4::GetUniformLocation(cubeProgram, "view");
	cubeProjLoc = gl4::GetUniformLocation(cubeProgram, "projection");

	texture = gl4::LoadTexture2D("CoreData/textures/colorful.png", true);

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
		{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{-0.5f,  0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
		{{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{ 0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
	};

	GLuint indices[] =
	{
		0, 2, 1,
		0, 3, 2
	};

	vbo = gl4::CreateBufferStorage(0, sizeof(vertices), vertices);
	ibo = gl4::CreateBufferStorage(0, sizeof(indices), indices);
	vao = gl4::CreateVertexArray(vbo, ibo, sizeof(Vertex), attribs);

	camera.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

	model = new Model("ExampleData/mesh/Sponza/Sponza.gltf");

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
void GameApp::OnRender()
{
	gl4::SetFrameBuffer(0, GetWidth(), GetHeight(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 matmodel = glm::mat4(1.0f);
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 proj = glm::perspective(glm::radians(60.0f), GetAspect(), 0.01f, 1000.0f);

	// вывод квада
	{
		glUseProgram(program);
		gl4::SetUniform(ModelLoc, matmodel);
		gl4::SetUniform(ViewLoc, view);
		gl4::SetUniform(ProjLoc, proj);

		glBindTextureUnit(0, texture);
		glBindVertexArray(vao);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	// вывод меша
	{
		glUseProgram(modelProgram);
		gl4::SetUniform(modelModelLoc, matmodel);
		gl4::SetUniform(modelViewLoc, view);
		gl4::SetUniform(modelProjLoc, proj);
		model->Draw(modelProgram);
	}

	// вывод кубов
	{
		glUseProgram(cubeProgram);
		gl4::SetUniform(cubeModelLoc, matmodel);
		gl4::SetUniform(cubeViewLoc, view);
		gl4::SetUniform(cubeProjLoc, proj);

		glBindTextureUnit(0, texture);
		// Render boxes
		for (unsigned int i = 0; i < 10; ++i)
		{
			// Calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model = glm::mat4(1.0f); // Make sure to initialize matrix to identity matrix first
			model = glm::translate(model, cubePositions[i]);
			const float angle = 20.0f * static_cast<float>(i);
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			model = glm::scale(model, glm::vec3(0.5));
			gl4::SetUniform(cubeModelLoc, model);

			GetGraphicSystem().DrawCube();
		}
	}
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