#include "stdafx.h"
#include "TestBloom.h"
//=============================================================================
namespace
{
	const char* lightShaderCodeVertex = R"(
#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 aPos;

layout(location = 0) out vec2 fragOffset;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 lightPosition;
uniform float radius;

const vec2 OFFSETS[6] = vec2[](
	vec2(-1.0, -1.0),
	vec2(-1.0,  1.0),
	vec2( 1.0, -1.0),
	vec2( 1.0, -1.0),
	vec2(-1.0,  1.0),
	vec2( 1.0,  1.0)
);

void main()
{
	vec3 camRight = vec3(view[0][0], view[1][0], view[2][0]);
	vec3 camUp = vec3(view[0][1], view[1][1], view[2][1]);

	fragOffset = OFFSETS[gl_VertexID];

	vec3 positionWorld = lightPosition +
		radius * fragOffset.x * camRight +
		radius * fragOffset.y * camUp;
	
	gl_Position = projection * view * vec4(positionWorld, 1.0);
}
)";

	const char* lightShaderCodeFragment = R"(
#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 fragOffset;
layout(location = 0) out vec4 FragColor;

uniform vec3 lightColor;

void main()
{
	float dist = sqrt(dot(fragOffset, fragOffset));
    //float dist = dot(fragOffset, fragOffset); // Remove sqrt
	if (dist >= 1.0)
	{
		discard;
	}
	// Blurry edge
	float alpha = 1.0 - pow(dist, 5.0);
	FragColor = vec4(normalize(lightColor), alpha);
}
)";

	gl4::ShaderProgram program;
	int lightPositionLoc;
	int radiusLoc;
	int lightColorLoc;

	Camera camera;

	Model* model;
	Light light(glm::vec3(0.0f, 0.5f, 5.0f), glm::vec3(1.f));

	auto modelRotation = acos(-1.f);
	unsigned int blurIter = 50;

	PipelineBloom* pipeline;
}
//=============================================================================
EngineConfig TestBloom::GetConfig() const
{
	return {};
}
//=============================================================================
bool TestBloom::OnCreate()
{
	program = gl4::CreateShaderProgram(lightShaderCodeVertex, lightShaderCodeFragment);
	lightPositionLoc = gl4::GetUniformLocation(program, "lightPosition");
	radiusLoc = gl4::GetUniformLocation(program, "radius");
	lightColorLoc = gl4::GetUniformLocation(program, "lightColor");

	model = new Model("ExampleData/mesh/Zaku/scene.gltf");

	pipeline = new PipelineBloom(blurIter);

	glClearColor(0.7f, 0.8f, 0.9f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	return true;
}
//=============================================================================
void TestBloom::OnDestroy()
{
	delete model;
	delete pipeline;
	glDeleteProgram(program);
}
//=============================================================================
void TestBloom::OnUpdate(float deltaTime)
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
void TestBloom::OnRender()
{
	gl4::SetFrameBuffer({ 0 }, GetWidth(), GetHeight(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 proj = glm::perspective(glm::radians(60.0f), GetAspect(), 0.01f, 1000.0f);


	// First pass
	pipeline->StartFirstPass(
		proj,
		view,
		camera.Position,
		light.Position
	);

	glm::mat4 modelMat = glm::mat4(1.0f);
	modelMat = glm::scale(modelMat, glm::vec3(1.0f, 1.0f, 1.0f));
	modelMat = glm::rotate(modelMat, modelRotation, glm::vec3(0.0f, 1.0f, 0.0f));
	auto mainShader = pipeline->GetMainShader();
	glUseProgram(mainShader);
	gl4::SetUniform(mainShader, "model", modelMat);
	model->Draw(mainShader);

	// End first pass
	pipeline->EndFirstPass();

	// Second pass
	pipeline->StartBlurPass();

	// Third pass
	pipeline->RenderComposite();

	// Light
	glUseProgram(program);
	gl4::SetUniform(program, "projection", proj);
	gl4::SetUniform(program, "view", view);
	gl4::SetUniform(lightPositionLoc, light.Position);
	gl4::SetUniform(radiusLoc, 0.4f);
	gl4::SetUniform(lightColorLoc, light.Color);
	GetGraphicSystem().DrawQuad();
}
//=============================================================================
void TestBloom::OnImGuiDraw()
{
}
//=============================================================================
void TestBloom::OnResize(uint16_t width, uint16_t height)
{
}
//=============================================================================