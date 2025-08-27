#include "stdafx.h"
#include "GameAppOld.h"
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



	gl::ShaderProgramId program;
	int ModelLoc;
	int ViewLoc;
	int ProjLoc;

	gl::ShaderProgramId modelProgram;
	int modelModelLoc;
	int modelViewLoc;
	int modelProjLoc;

	gl::ShaderProgramId cubeProgram;
	int cubeModelLoc;
	int cubeViewLoc;
	int cubeProjLoc;

	GLuint texture;
	gl::BufferId vbo;
	gl::BufferId ibo;
	gl::VertexArrayId vao;

	Camera camera;

	ModelOLD* model;

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
EngineCreateInfo GameAppOld::GetCreateInfo() const
{
	return {};
}
//=============================================================================
bool GameAppOld::OnInit()
{
	program = gl::CreateShaderProgram(shaderCodeVertex, shaderCodeFragment);
	ModelLoc = gl::GetUniformLocation(program, "Model");
	ViewLoc = gl::GetUniformLocation(program, "View");
	ProjLoc = gl::GetUniformLocation(program, "Proj");

	modelProgram = gl::CreateShaderProgram(modelShaderCodeVertex, modelShaderCodeFragment);
	modelModelLoc = gl::GetUniformLocation(modelProgram, "model");
	modelViewLoc = gl::GetUniformLocation(modelProgram, "view");
	modelProjLoc = gl::GetUniformLocation(modelProgram, "projection");

	cubeProgram = gl::CreateShaderProgram(cubeShaderCodVertex, cubeShaderCodeFragment);
	cubeModelLoc = gl::GetUniformLocation(cubeProgram, "model");
	cubeViewLoc = gl::GetUniformLocation(cubeProgram, "view");
	cubeProjLoc = gl::GetUniformLocation(cubeProgram, "projection");

	texture = gl::LoadTexture2D("CoreData/textures/colorful.png", true);

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 uv;
	};

	std::vector<gl::VertexAttributeRaw> attribs = {
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

	vbo = gl::CreateBufferStorage(0, sizeof(vertices), vertices);
	ibo = gl::CreateBufferStorage(0, sizeof(indices), indices);
	vao = gl::CreateVertexArray(vbo, ibo, sizeof(Vertex), attribs);

	camera.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

	model = new ModelOLD("ExampleData/mesh/Sponza/Sponza.gltf");

	glClearColor(0.7f, 0.8f, 0.9f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	gr.Create();

	return true;
}
//=============================================================================
void GameAppOld::OnClose()
{
	gr.Destroy();
	glDeleteTextures(1, &texture);
	gl::Destroy(program);
	gl::Destroy(vbo);
	gl::Destroy(ibo);
	gl::Destroy(vao);
}
//=============================================================================
void GameAppOld::OnUpdate(float deltaTime)
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
		Input::SetCursorVisible(false);
		camera.ProcessMouseMovement(Input::GetCursorOffset().x, Input::GetCursorOffset().y);
	}
	else if (glfwGetMouseButton(GetGLFWWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
	{
		glfwSetInputMode(GetGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		Input::SetCursorVisible(true);
	}
}
//=============================================================================
void GameAppOld::OnRender()
{
	gl::SetFrameBuffer({ 0 }, GetWindowWidth(), GetWindowHeight(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 matmodel = glm::mat4(1.0f);
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 proj = glm::perspective(glm::radians(60.0f), GetWindowAspect(), 0.01f, 1000.0f);

	// вывод квада
	{
		glUseProgram(program);
		gl::SetUniform(program, ModelLoc, matmodel);
		gl::SetUniform(program, ViewLoc, view);
		gl::SetUniform(program, ProjLoc, proj);

		glBindTextureUnit(0, texture);
		glBindVertexArray(vao);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	// вывод меша
	{
		glUseProgram(modelProgram);
		gl::SetUniform(modelProgram, modelModelLoc, matmodel);
		gl::SetUniform(modelProgram, modelViewLoc, view);
		gl::SetUniform(modelProgram, modelProjLoc, proj);
		model->Draw(modelProgram);
	}

	// вывод кубов
	{
		glUseProgram(cubeProgram);
		gl::SetUniform(cubeProgram, cubeModelLoc, matmodel);
		gl::SetUniform(cubeProgram, cubeViewLoc, view);
		gl::SetUniform(cubeProgram, cubeProjLoc, proj);

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
			gl::SetUniform(cubeProgram, cubeModelLoc, model);

			gr.DrawCube();
		}
	}
}
//=============================================================================
void GameAppOld::OnImGuiDraw()
{
	DrawProfilerInfo();
}
//=============================================================================
void GameAppOld::OnResize(uint16_t width, uint16_t height)
{

}
//=============================================================================
void GameAppOld::OnMouseButton(int button, int action, int mods)
{
}
//=============================================================================
void GameAppOld::OnMousePos(double x, double y)
{
}
//=============================================================================
void GameAppOld::OnScroll(double dx, double dy)
{
}
//=============================================================================
void GameAppOld::OnKey(int key, int scanCode, int action, int mods)
{
}
//=============================================================================