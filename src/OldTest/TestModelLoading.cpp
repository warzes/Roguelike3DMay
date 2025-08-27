#include "stdafx.h"
#include "TestModelLoading.h"
//=============================================================================
namespace
{
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
uniform vec3 lightPos;
uniform vec3 viewPos;

// Blinn-Phong with glTF model
void main()
{
	vec3 color = texture(texture_diffuse1, fs_in.TexCoords).rgb;
	
	// ambient
	vec3 ambient = 0.05 * color;
	
	// diffuse
	vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	vec3 normal = normalize(fs_in.Normal);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * color;
	
	// specular
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = 0.0;

	vec3 halfwayDir = normalize(lightDir + viewDir);  
	spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

	vec3 specular = vec3(0.3) * spec; // assuming bright white light color
	FragColor = vec4(ambient + diffuse + specular, 1.0);
}
)";

	const char* lightShaderCodeVertex = R"(
#version 460 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";
	const char* lightShaderCodeFragment = R"(
#version 460 core
out vec4 FragColor;

void main()
{
	FragColor = vec4(1.0); // set all 4 vector values to 1.0
}
)";


	gl::ShaderProgramId modelProgram;
	int modelModelLoc;
	int modelViewLoc;
	int modelProjLoc;
	int lightPosLoc;
	int viewPosLoc;

	gl::ShaderProgramId lightProgram;
	int lightModelLoc;
	int lightViewLoc;
	int lightProjLoc;


	Camera camera;

	ModelOLD* model;

	const glm::vec3 lightPos(0.0f, 0.5f, 5.0f);
	auto modelRotation = 0.0f;
}
//=============================================================================
EngineCreateInfo TestModelLoading::GetCreateInfo() const
{
	return {};
}
//=============================================================================
bool TestModelLoading::OnInit()
{
	modelProgram = gl::CreateShaderProgram(modelShaderCodeVertex, modelShaderCodeFragment);
	modelModelLoc = gl::GetUniformLocation(modelProgram, "model");
	modelViewLoc = gl::GetUniformLocation(modelProgram, "view");
	modelProjLoc = gl::GetUniformLocation(modelProgram, "projection");
	lightPosLoc = gl::GetUniformLocation(modelProgram, "lightPos");
	viewPosLoc = gl::GetUniformLocation(modelProgram, "viewPos");

	lightProgram = gl::CreateShaderProgram(lightShaderCodeVertex, lightShaderCodeFragment);
	lightModelLoc = gl::GetUniformLocation(lightProgram, "model");
	lightViewLoc = gl::GetUniformLocation(lightProgram, "view");
	lightProjLoc = gl::GetUniformLocation(lightProgram, "projection");


	model = new ModelOLD("ExampleData/mesh/Tachikoma/Tachikoma.gltf");

	camera.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

	glClearColor(0.7f, 0.8f, 0.9f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	gr.Create();

	return true;
}
//=============================================================================
void TestModelLoading::OnClose()
{
	gr.Destroy();
}
//=============================================================================
void TestModelLoading::OnUpdate(float deltaTime)
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
void TestModelLoading::OnRender()
{
	gl::SetFrameBuffer({ 0 }, GetWindowWidth(), GetWindowHeight(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 proj = glm::perspective(glm::radians(60.0f), GetWindowAspect(), 0.01f, 1000.0f);

	// вывод модели
	{
		glUseProgram(modelProgram);
		gl::SetUniform(modelProgram, modelProjLoc, proj);
		gl::SetUniform(modelProgram, modelViewLoc, view);
		gl::SetUniform(modelProgram, viewPosLoc, camera.Position);
		gl::SetUniform(modelProgram, lightPosLoc, lightPos);

		glm::mat4 modelMat = glm::mat4(1.0f);
		modelMat = glm::scale(modelMat, glm::vec3(1.0f, 1.0f, 1.0f));
		modelMat = glm::rotate(modelMat, modelRotation, glm::vec3(0.0f, 1.0f, 0.0f));
		modelRotation += GetDeltaTime();
		gl::SetUniform(modelProgram, modelModelLoc, modelMat);

		model->Draw(modelProgram);
	}

	// рендер источника света
	{
		glUseProgram(lightProgram);
		gl::SetUniform(lightProgram, lightProjLoc, proj);
		gl::SetUniform(lightProgram, lightViewLoc, view);

		glm::mat4 modelMat = glm::mat4(1.0f);
		modelMat = glm::translate(modelMat, lightPos);
		modelMat = glm::scale(modelMat, glm::vec3(0.2f));
		gl::SetUniform(lightProgram, lightModelLoc, modelMat);

		gr.DrawCube();
	}
}
//=============================================================================
void TestModelLoading::OnImGuiDraw()
{
}
//=============================================================================
void TestModelLoading::OnResize(uint16_t width, uint16_t height)
{
}
//=============================================================================