#include "stdafx.h"
#include "TestSkyboxWithCube.h"
//=============================================================================
namespace
{
	const char* cubeShaderVertex = R"(
#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 Normal;
out vec3 Position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	TexCoords = aTexCoords;
	Normal = transpose(inverse(mat3(model))) * aNormal;
	Position = vec3(model * vec4(aPos, 1.0));
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

	const char* cubeShaderFragment = R"(
#version 460 core

out vec4 FragColor;

in vec3 Position;
in vec2 TexCoords;
in vec3 Normal;

layout(binding = 0) uniform samplerCube skybox;
layout(binding = 1) uniform sampler2D texture1;
uniform vec3 cameraPos;

void main()
{
	vec3 I = normalize(Position - cameraPos);
	vec3 R = reflect(I, normalize(Normal));
	vec3 skyColor = texture(skybox, R).rgb;
	vec3 texColor = texture(texture1, TexCoords).rgb;
	vec3 finalColor = (skyColor * 0.9) + (texColor * 0.1);
	FragColor = vec4(finalColor, 1.0);
}
)";

	const char* skyboxShaderVertex = R"(
#version 460 core

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
	TexCoords = aPos;
	vec4 pos = projection * view * vec4(aPos, 1.0);
	gl_Position = pos.xyww;
}
)";
	const char* skyboxShaderFragment = R"(
#version 460 core

out vec4 FragColor;

in vec3 TexCoords;

layout(binding = 0) uniform samplerCube skybox;

void main()
{
	FragColor = texture(skybox, TexCoords);
}
)";


	gl4::ShaderProgram cubeProgram;
	int cubeModelLoc;
	int cubeViewLoc;
	int cubeProjectionLoc;
	int cubeCameraPosLoc;

	gl4::ShaderProgram skyboxProgram;
	int skyViewLoc;
	int skyProjectionLoc;

	GLuint skyboxTexture;
	GLuint cubeTexture;

	Camera camera;
}
//=============================================================================
EngineConfig TestSkyboxWithCube::GetConfig() const
{
	return {};
}
//=============================================================================
bool TestSkyboxWithCube::OnCreate()
{
	cubeProgram = gl4::CreateShaderProgram(cubeShaderVertex, cubeShaderFragment);
	cubeModelLoc = gl4::GetUniformLocation(cubeProgram, "model");
	cubeViewLoc = gl4::GetUniformLocation(cubeProgram, "view");
	cubeProjectionLoc = gl4::GetUniformLocation(cubeProgram, "projection");
	cubeCameraPosLoc = gl4::GetUniformLocation(cubeProgram, "cameraPos");

	skyboxProgram = gl4::CreateShaderProgram(skyboxShaderVertex, skyboxShaderFragment);
	skyViewLoc = gl4::GetUniformLocation(skyboxProgram, "view");
	skyProjectionLoc = gl4::GetUniformLocation(skyboxProgram, "projection");

	cubeTexture = gl4::LoadTexture2D("CoreData/textures/colorful.png", true);

	const std::vector<std::string> files
	{
		"right.png",
		"left.png",
		"top.png",
		"bottom.png",
		"front.png",
		"back.png"
	};
	skyboxTexture = gl4::LoadCubeMap(files, "ExampleData/textures/skybox_blue_space/");
	
	camera.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

	glClearColor(0.7f, 0.8f, 0.9f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	return true;
}
//=============================================================================
void TestSkyboxWithCube::OnDestroy()
{

}
//=============================================================================
void TestSkyboxWithCube::OnUpdate(float deltaTime)
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
void TestSkyboxWithCube::OnRender()
{
	gl4::SetFrameBuffer(0, GetWidth(), GetHeight(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 proj = glm::perspective(glm::radians(60.0f), GetAspect(), 0.01f, 1000.0f);

	// Draw Cube
	{
		glUseProgram(cubeProgram);
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.5f));
		gl4::SetUniform(cubeModelLoc, model);
		gl4::SetUniform(cubeViewLoc, view);
		gl4::SetUniform(cubeProjectionLoc, proj);
		gl4::SetUniform(cubeCameraPosLoc, camera.Position);
		glBindTextureUnit(0, skyboxTexture);
		glBindTextureUnit(1, cubeTexture);
		GetGraphicSystem().DrawCube();
		glBindVertexArray(0);
	}

	// Draw skybox
	{
		glDepthFunc(GL_LEQUAL); // Change depth function so depth test passes when values are equal to depth buffer's content

		glUseProgram(skyboxProgram);
		auto skyboxView = glm::mat4(glm::mat3(view)); // Remove translation from the view matrix
		gl4::SetUniform(skyViewLoc, skyboxView);
		gl4::SetUniform(skyProjectionLoc, proj);
		glBindTextureUnit(0, skyboxTexture);
		GetGraphicSystem().DrawCube();
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // Set depth function back to default
	}
}
//=============================================================================
void TestSkyboxWithCube::OnImGuiDraw()
{
}
//=============================================================================
void TestSkyboxWithCube::OnResize(uint16_t width, uint16_t height)
{
}
//=============================================================================