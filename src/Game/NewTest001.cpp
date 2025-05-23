#include "stdafx.h"
#include "NewTest001.h"
//=============================================================================
namespace
{
	const char* shaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec3 a_color;

layout(location = 0) out vec3 v_color;

void main()
{
  v_color = a_color;
  gl_Position = vec4(a_pos, 0.0, 1.0);
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

layout(location = 0) out vec4 o_color;

layout(location = 0) in vec3 v_color;

void main()
{
  o_color = vec4(v_color, 1.0);
}
)";

	gl4::BufferStorageId vertexPosBuffer;
	gl4::BufferStorageId vertexColorBuffer;
	gl4::GraphicsPipelineId pipeline;

	gl4::GraphicsPipelineId CreatePipeline()
	{
		auto descPos = gl4::VertexInputBindingDescription{
		  .location = 0,
		  .binding = 0,
		  .format = gl4::Format::R32G32_FLOAT,
		  .offset = 0,
		};
		auto descColor = gl4::VertexInputBindingDescription{
		  .location = 1,
		  .binding = 1,
		  .format = gl4::Format::R8G8B8_UNORM,
		  .offset = 0,
		};
		auto inputDescs = { descPos, descColor };

		return gl4::CreateGraphicsPipeline({
			  .debugName = "Triangle Pipeline",
			  .vertexShader = shaderCodeVertex,
			  .fragmentShader = shaderCodeFragment,
			  .inputAssemblyState = {.topology = gl4::PrimitiveTopology::TRIANGLE_LIST},
			  .vertexInputState = {inputDescs},
		});
	}
}
//=============================================================================
EngineConfig NewTest001::GetConfig() const
{
	return {};
}
//=============================================================================
bool NewTest001::OnCreate()
{
	static constexpr std::array<float, 6> triPositions = { -0, -0, 1, -1, 1, 1 };
	static constexpr std::array<uint8_t, 9> triColors = { 255, 0, 0, 0, 255, 0, 0, 0, 255 };
	vertexPosBuffer = gl4::CreateStorageBuffer(triPositions);
	vertexColorBuffer = gl4::CreateStorageBuffer(triColors);
	pipeline = CreatePipeline();

	glClearColor(0.7f, 0.8f, 0.9f, 1.0f);
	//glEnable(GL_DEPTH_TEST);

	return true;
}
//=============================================================================
void NewTest001::OnDestroy()
{
	gl4::Destroy(vertexPosBuffer);
	gl4::Destroy(vertexColorBuffer);
	gl4::Destroy(pipeline);
}
//=============================================================================
void NewTest001::OnUpdate(float deltaTime)
{
}
//=============================================================================
void NewTest001::OnRender()
{
	gl4::SetFrameBuffer({ 0 }, GetWidth(), GetHeight(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // TODO: заменить

	gl4::Cmd::BindGraphicsPipeline(pipeline);

	gl4::Cmd::BindVertexBuffer(0, vertexPosBuffer, 0, 2 * sizeof(float));
	gl4::Cmd::BindVertexBuffer(1, vertexColorBuffer, 0, 3 * sizeof(uint8_t));

	gl4::Cmd::Draw(3, 1, 0, 0);
}
//=============================================================================
void NewTest001::OnImGuiDraw()
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
void NewTest001::OnResize(uint16_t width, uint16_t height)
{
}
//=============================================================================