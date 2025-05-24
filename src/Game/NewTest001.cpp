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

	std::optional<gl4::Buffer> vertexPosBuffer;
	std::optional<gl4::Buffer> vertexColorBuffer;
	std::optional<gl4::GraphicsPipeline> pipeline;

	gl4::GraphicsPipeline CreatePipeline()
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

		auto vertexShader = gl4::Shader(gl4::PipelineStage::VertexShader, shaderCodeVertex, "Triangle VS");
		auto fragmentShader = gl4::Shader(gl4::PipelineStage::FragmentShader, shaderCodeFragment, "Triangle FS");

		return gl4::GraphicsPipeline({
			 .name = "Triangle Pipeline",
			.vertexShader = &vertexShader,
			.fragmentShader = &fragmentShader,
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
	vertexPosBuffer = gl4::Buffer(triPositions);
	vertexColorBuffer = gl4::Buffer(triColors);
	pipeline = CreatePipeline();

	glClearColor(0.7f, 0.8f, 0.9f, 1.0f);
	//glEnable(GL_DEPTH_TEST);

	return true;
}
//=============================================================================
void NewTest001::OnDestroy()
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
	// Before we are allowed to render anything, we must declare what we are rendering to.
	  // In this case we are rendering straight to the screen, so we can use RenderToSwapchain.
	  // We are also provided with an opportunity to clear any of the render targets here (by setting the load op to clear).
	  // We will use it to clear the color buffer with a soothing dark magenta.
	gl4::RenderToSwapchain(
		gl4::SwapchainRenderInfo{
		  .name = "Render Triangle",
		  .viewport = gl4::Viewport{.drawRect{.offset = {0, 0}, .extent = {GetWindowWidth(), GetWindowHeight()}}},
		  .colorLoadOp = gl4::AttachmentLoadOp::Clear,
		  .clearColorValue = {.2f, .0f, .2f, 1.0f},
		},
		[&]
		{
			// Functions in Cmd can only be called inside a rendering (Render() or RenderToSwapchain()) or compute scope (Compute()).
			// Pipelines must be bound before we can issue drawing-related calls.
			// This is where, under the hood, the actual GL program is bound and all the pipeline state is set.
			gl4::Cmd::BindGraphicsPipeline(pipeline.value());

			// Vertex buffers are bound at draw time, similar to Vulkan or with glBindVertexBuffer.
			gl4::Cmd::BindVertexBuffer(0, vertexPosBuffer.value(), 0, 2 * sizeof(float));
			gl4::Cmd::BindVertexBuffer(1, vertexColorBuffer.value(), 0, 3 * sizeof(uint8_t));

			// Let's draw 1 instance with 3 vertices.
			gl4::Cmd::Draw(3, 1, 0, 0);
		});
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