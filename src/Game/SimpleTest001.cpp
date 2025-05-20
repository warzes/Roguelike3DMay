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

	gl4::ShaderProgramId program;

	MVPData mvpData;
	GLuint mvpUbo;

	GLuint texture;

	gl4f::Buffer* vertexPosBuffer;
	gl4f::Buffer* vertexColorBuffer;
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

	static constexpr std::array<float, 6> triPositions = { -0, -0, 1, -1, 1, 1 };
	static constexpr std::array<uint8_t, 9> triColors = { 255, 0, 0, 0, 255, 0, 0, 0, 255 };
	vertexPosBuffer = new gl4f::Buffer(triPositions);
	vertexColorBuffer = new gl4f::Buffer(triColors);

	glClearColor(0.7f, 0.8f, 0.9f, 1.0f);

	return true;
}
//=============================================================================
void SimpleTest001::OnDestroy()
{
}
//=============================================================================
void SimpleTest001::OnUpdate(float deltaTime)
{
}
//=============================================================================
void SimpleTest001::OnRender()
{
	gl4::SetFrameBuffer({ 0 }, GetWidth(), GetHeight(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mvpData.model = glm::mat4(1.0f);
	mvpData.projection = glm::perspective(glm::radians(60.0f), GetAspect(), 0.01f, 1000.0f);

	glNamedBufferSubData(mvpUbo, 0, sizeof(MVPData), &mvpData);
	
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