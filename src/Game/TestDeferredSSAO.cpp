#include "stdafx.h"
#include "TestDeferredSSAO.h"
//=============================================================================
namespace
{
	const char* lightSphereShaderVertex = R"(
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

	const char* lightSphereShaderFragment = R"(
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

	Camera camera;

	GLuint lightSphereShader;

	std::vector<Light> lights{};
	std::vector<float> lightAngles{};
	std::vector<float> lightRadii{};

	Model* model;

	PipelineDeferredSSAO* pipeline;
	constexpr int noiseSize = 4;
	constexpr int kernelSize = 64;
	float radius = 0.5f;
	float bias = 0.025f;
}
//=============================================================================
EngineConfig TestDeferredSSAO::GetConfig() const
{
	return {};
}
//=============================================================================
bool TestDeferredSSAO::OnCreate()
{	
	camera.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

	pipeline = new PipelineDeferredSSAO(kernelSize, noiseSize);

	InitScene();
	InitLights();

	glClearColor(0.7f, 0.8f, 0.9f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	return true;
}
//=============================================================================
void TestDeferredSSAO::OnDestroy()
{
	delete pipeline;
}
//=============================================================================
void TestDeferredSSAO::OnUpdate(float deltaTime)
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
void TestDeferredSSAO::OnRender()
{
	gl4::SetFrameBuffer(0, GetWidth(), GetHeight(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 proj = glm::perspective(glm::radians(60.0f), GetAspect(), 0.01f, 1000.0f);

	// 1 Geometry pass: render scene's geometry/color data into G buffer
	pipeline->StartGeometryPass(proj, view);
	RenderScene(pipeline->GetGeometryShader());
	pipeline->EndGeometryPass();

	// 2 SSAO
	pipeline->StartSSAOPass(proj, kernelSize, radius, bias);

	// 3 Blur SSAO texture to remove noise
	pipeline->StartBlurPass();

	UpdateLightPositions();

	// 4 lighting pass: traditional deferred Blinn-Phong lighting with added screen-space ambient occlusion
	pipeline->StartLightingPass(lights, view, camera.Position);

	// 5 Blit
	pipeline->Blit();

	RenderLights();
}
//=============================================================================
void TestDeferredSSAO::OnImGuiDraw()
{
	ImGui::SetNextWindowSize(ImVec2(500, 100));

	ImGui::Begin("SSAO");

	ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Vendor: %s", (char*)glGetString(GL_VENDOR));
	ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Version: %s", (char*)glGetString(GL_VERSION));
	ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Renderer: %s", (char*)glGetString(GL_RENDERER));
	ImGui::Separator();

	ImGui::Text("Framerate: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::SliderFloat("Radius", &radius, 0.0f, 50.0f);
	ImGui::SliderFloat("Bias", &bias, 0.0f, 0.5f);

	ImGui::End();
}
//=============================================================================
void TestDeferredSSAO::OnResize(uint16_t width, uint16_t height)
{
}
//=============================================================================
void TestDeferredSSAO::InitScene()
{
	model = new Model("data/mesh/Sponza/Sponza.gltf");
}
//=============================================================================
void TestDeferredSSAO::UpdateLightPositions()
{
	for (unsigned int i = 0; i < lights.size(); ++i)
	{
		const float step = GetDeltaTime() * 0.2f;
		const float yPos = lights[i].Position.y;
		lightAngles[i] += step;
		lights[i].Position = glm::vec3(
			glm::cos(lightAngles[i]) * lightRadii[i],
			yPos,
			glm::sin(lightAngles[i]) * lightRadii[i]
		);
	}
}
//=============================================================================
void TestDeferredSSAO::InitLights()
{
	lightSphereShader = gl4::CreateShaderProgram(lightSphereShaderVertex, lightSphereShaderFragment);
	const float pi2 = glm::two_pi<float>();
	constexpr uint32_t NR_LIGHTS = 200;
	for (uint32_t i = 0; i < NR_LIGHTS; ++i)
	{
		const float yPos = RandomNumber<float>(0.15f, 10.0f);
		const float radius = RandomNumber<float>(5.0f, 15.0f);
		const float rad = RandomNumber<float>(0.0f, pi2);
		float xPos = glm::cos(rad);

		glm::vec3 position(
			glm::cos(rad) * radius,
			yPos,
			glm::sin(rad) * radius
		);

		glm::vec3 color(
			RandomNumber<float>(0.5f, 0.8f),
			RandomNumber<float>(0.5f, 0.8f),
			RandomNumber<float>(0.7f, 1.0f)
		);

		lightAngles.push_back(rad);
		lightRadii.push_back(radius);
		lights.emplace_back(position, color);
	}

	glUseProgram(lightSphereShader);
	gl4::SetUniform(lightSphereShader, "radius", 0.2f);
}
//=============================================================================
void TestDeferredSSAO::RenderLights()
{
	glm::mat4 proj = glm::perspective(glm::radians(60.0f), GetAspect(), 0.01f, 1000.0f);

	glUseProgram(lightSphereShader);
	gl4::SetUniform(lightSphereShader, "projection", proj);
	gl4::SetUniform(lightSphereShader, "view", camera.GetViewMatrix());
	for (const Light& light : lights)
	{
		gl4::SetUniform(lightSphereShader, "lightPosition", light.Position);
		gl4::SetUniform(lightSphereShader, "radius", 0.4f);
		gl4::SetUniform(lightSphereShader, "lightColor", light.Color);
		GetGraphicSystem().DrawQuad();
	}
}
//=============================================================================
void TestDeferredSSAO::RenderScene(GLuint shader) const
{
	glm::mat4 modelMat = glm::mat4(1.0f);
	modelMat = glm::translate(modelMat, glm::vec3(8.0f, 0.0f, 1.0f));
	modelMat = glm::scale(modelMat, glm::vec3(2.0f));
	glUseProgram(shader);
	gl4::SetUniform(shader, "model", modelMat);
	model->Draw(shader);
}
//=============================================================================