#include "stdafx.h"
#include "SimpleTest001.h"
/* 01_hello_triangle
 *
 * This example renders a simple triangle.
 *
 * Shown:
 * + Creating vertex buffers
 * + Specifying vertex attributes
 * + Loading shaders
 * + Creating a graphics pipeline
 * + Rendering to the screen
 */
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

	static gl4::GraphicsPipeline CreatePipeline()
	{
		// Specify our two vertex attributes: position and color.
		// Positions are 2x float, so we will use R32G32_FLOAT like we would in Vulkan.
		auto descPos = gl4::VertexInputBindingDescription{
		  .location = 0,
		  .binding = 0,
		  .format = gl4::Format::R32G32_FLOAT,
		  .offset = 0,
		};
		// Colors are 3x uint8, so we will use R8G8B8.
		// To treat them as normalized floats in [0, 1], we make sure it's a _UNORM format.
		// This means we do not need to specify whether the data is normalized like we would with glVertexAttribPointer.
		auto descColor = gl4::VertexInputBindingDescription{
		  .location = 1,
		  .binding = 1,
		  .format = gl4::Format::R8G8B8_UNORM,
		  .offset = 0,
		};
		// Create an initializer list or array (or anything implicitly convertable to std::span)
		// of our input binding descriptions to send to the pipeline.
		auto inputDescs = { descPos, descColor };

		// We compile our shaders here. Just provide the shader source string and you are good to go!
		// Note that the shaders are compiled here and throw ShaderCompilationException if they fail.
		// The compiler's error message will be stored in the exception.
		// In a real application we might handle these exceptions, but here we will let them propagate up.
		auto vertexShader = gl4::Shader(gl4::PipelineStage::VERTEX_SHADER, shaderCodeVertex, "Triangle VS");
		auto fragmentShader = gl4::Shader(gl4::PipelineStage::FRAGMENT_SHADER, shaderCodeFragment, "Triangle FS");

		// The graphics pipeline contains all the state necessary for rendering.
		// It is self-contained, immutable, and isolated from other pipelines' state (state leaking cannot happen).
		// Here we give it our two shaders and the input binding descriptions.
		// We could specify a lot more state if we wanted, but for this simple example the defaults will suffice.
		// Note that compiling a pipeline will link its non-null shaders together.
		// If linking fails, a PipelineCompilationException containing the linker error will be thrown.
		// Similar to before, we will let possible exceptions propagate up.
		return gl4::GraphicsPipeline{ {
		  .name = "Triangle Pipeline",
		  .vertexShader = &vertexShader,
		  .fragmentShader = &fragmentShader,
		  .inputAssemblyState = {.topology = gl4::PrimitiveTopology::TRIANGLE_LIST},
		  .vertexInputState = {inputDescs},
		} };
	}
}
//=============================================================================
EngineConfig SimpleTest001::GetConfig() const
{
	return {};
}
//=============================================================================
bool SimpleTest001::OnCreate()
{
	static constexpr std::array<float, 6> triPositions = { -0, -0, 1, -1, 1, 1 };
	static constexpr std::array<uint8_t, 9> triColors = { 255, 0, 0, 0, 255, 0, 0, 0, 255 };
	vertexPosBuffer = gl4::Buffer(triPositions);
	vertexColorBuffer = gl4::Buffer(triColors);
	pipeline = CreatePipeline();

	glClearColor(0.7f, 0.8f, 0.9f, 1.0f);

	return true;
}
//=============================================================================
void SimpleTest001::OnDestroy()
{
	pipeline = std::nullopt;
}
//=============================================================================
void SimpleTest001::OnUpdate(float deltaTime)
{
}
//=============================================================================
void SimpleTest001::OnRender()
{
	// Before we are allowed to render anything, we must declare what we are rendering to.
	// In this case we are rendering straight to the screen, so we can use RenderToSwapchain.
	// We are also provided with an opportunity to clear any of the render targets here (by setting the load op to clear).
	// We will use it to clear the color buffer with a soothing dark magenta.
	gl4::RenderToSwapchain(
		gl4::SwapchainRenderInfo{
		  .name = "Render Triangle",
		  .viewport = gl4::Viewport{.drawRect{.offset = {0, 0}, .extent = {GetWidth(), GetHeight()}}},
		  .colorLoadOp = gl4::AttachmentLoadOp::CLEAR,
		  .clearColorValue = {.2f, .0f, .2f, 1.0f},
		},
		[&]
		{
			// Functions in Cmd can only be called inside a rendering (Render() or RenderToSwapchain()) or compute scope (Compute()).
			// Pipelines must be bound before we can issue drawing-related calls.
			// This is where, under the hood, the actual GL program is bound and all the pipeline state is set.
			gl4::Cmd::BindGraphicsPipeline(*pipeline);

			// Vertex buffers are bound at draw time, similar to Vulkan or with glBindVertexBuffer.
			gl4::Cmd::BindVertexBuffer(0, *vertexPosBuffer, 0, 2 * sizeof(float));
			gl4::Cmd::BindVertexBuffer(1, *vertexColorBuffer, 0, 3 * sizeof(uint8_t));

			// Let's draw 1 instance with 3 vertices.
			gl4::Cmd::Draw(3, 1, 0, 0);
		});
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