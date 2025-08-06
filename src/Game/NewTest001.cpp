#include "stdafx.h"
#include "NewTest001.h"
//=============================================================================
// Вывод треугольника
// - разделенные вершинные буферы - один для позиции, другой для цвета.
// - буфер цвета целочисленный из трех uint8_t
// - простой GraphicsPipeline
// - вывод сразу на экран (SwapChain)
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

	std::optional<gl::Buffer> vertexPosBuffer;
	std::optional<gl::Buffer> vertexColorBuffer;
	std::optional<gl::GraphicsPipeline> pipeline;

	gl::GraphicsPipeline CreatePipeline()
	{
		auto descPos = gl::VertexInputBindingDescription{
		  .location = 0,
		  .binding = 0,
		  .format = gl::Format::R32G32_FLOAT,
		  .offset = 0,
		};
		auto descColor = gl::VertexInputBindingDescription{
		  .location = 1,
		  .binding = 1,
		  .format = gl::Format::R8G8B8_UNORM,
		  .offset = 0,
		};
		auto inputDescs = { descPos, descColor };

		auto vertexShader = gl::Shader(gl::PipelineStage::VertexShader, shaderCodeVertex, "Triangle VS");
		auto fragmentShader = gl::Shader(gl::PipelineStage::FragmentShader, shaderCodeFragment, "Triangle FS");

		return gl::GraphicsPipeline({
			 .name = "Triangle Pipeline",
			.vertexShader = &vertexShader,
			.fragmentShader = &fragmentShader,
			.inputAssemblyState = {.topology = gl::PrimitiveTopology::TriangleList},
			.vertexInputState = {inputDescs},
		});
	}
}
//=============================================================================
EngineCreateInfo NewTest001::GetCreateInfo() const
{
	return {};
}
//=============================================================================
bool NewTest001::OnInit()
{
	static constexpr std::array<float, 6> triPositions = { 
		 0.0f,  0.4f, 
		-1.0f, -1.0f, 
		 1.0f, -1.0f };
	static constexpr std::array<uint8_t, 9> triColors = { 255, 0, 0, 0, 255, 0, 0, 0, 255 };
	vertexPosBuffer = gl::Buffer(triPositions);
	vertexColorBuffer = gl::Buffer(triColors);
	pipeline = CreatePipeline();

	return true;
}
//=============================================================================
void NewTest001::OnClose()
{
	vertexPosBuffer = {};
	vertexColorBuffer = {};
	pipeline = {};
}
//=============================================================================
void NewTest001::OnUpdate(float deltaTime)
{
}
//=============================================================================
void NewTest001::OnRender()
{
	const gl::SwapchainRenderInfo renderInfo
	{
		.name = "Render Triangle",
		.viewport = {.drawRect{.offset = {0, 0}, .extent = {GetWindowWidth(), GetWindowHeight()}}},
		.colorLoadOp = gl::AttachmentLoadOp::Clear,
		.clearColorValue = {.1f, .5f, .8f, 1.0f},
	};

	gl::BeginSwapChainRendering(renderInfo);
	{
		gl::Cmd::BindGraphicsPipeline(pipeline.value());
		gl::Cmd::BindVertexBuffer(0, vertexPosBuffer.value(), 0, 2 * sizeof(float));
		gl::Cmd::BindVertexBuffer(1, vertexColorBuffer.value(), 0, 3 * sizeof(uint8_t));
		gl::Cmd::Draw(3, 1, 0, 0);
	}
	gl::EndRendering();
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
void NewTest001::OnMouseButton(int button, int action, int mods)
{
}
//=============================================================================
void NewTest001::OnMousePos(double x, double y)
{
}
//=============================================================================
void NewTest001::OnScroll(double dx, double dy)
{
}
//=============================================================================
void NewTest001::OnKey(int key, int scanCode, int action, int mods)
{
}
//=============================================================================