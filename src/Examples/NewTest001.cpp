#include "stdafx.h"
#include "NewTest001.h"

//=============================================================================
// Вывод треугольника на основную поверхность
// - вершинный буфер из позиции и цвета
// - индексный буфер
// - создание GraphicsPipeline
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
	gl_Position = vec4(a_pos.xy, 0.0, 1.0);
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

	struct Vertex final
	{
		glm::vec2 pos;
		glm::vec3 color;
	};

	constexpr std::array<gl::VertexInputBindingDescription, 2> inputBindingDescs{
		gl::VertexInputBindingDescription{
			.location = 0,
			.binding = 0,
			.format = gl::Format::R32G32_FLOAT,
			.offset = offsetof(Vertex, pos),
		},
		gl::VertexInputBindingDescription{
			.location = 1,
			.binding = 0,
			.format = gl::Format::R32G32B32_FLOAT,
			.offset = offsetof(Vertex, color),
		},
	};

	std::optional<gl::Buffer> vertexBuffer;
	std::optional<gl::Buffer> indexBuffer;
	std::optional<gl::GraphicsPipeline> pipeline;

	gl::GraphicsPipeline CreatePipeline()
	{
		auto vertexShader = gl::Shader(gl::ShaderType::VertexShader, shaderCodeVertex, "Triangle VS");
		auto fragmentShader = gl::Shader(gl::ShaderType::FragmentShader, shaderCodeFragment, "Triangle FS");

		return gl::GraphicsPipeline({
			 .name = "Triangle Pipeline",
			.vertexShader = &vertexShader,
			.fragmentShader = &fragmentShader,
			.inputAssemblyState = {.topology = gl::PrimitiveTopology::TriangleList},
			.vertexInputState = {inputBindingDescs},
			});
	}

	void resize(uint16_t width, uint16_t height)
	{
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
	std::vector<Vertex> v = {
		{{  0.0f,  0.4f}, {1, 0, 0}},
		{{ -1.0f, -1.0f}, {0, 1, 0}},
		{{  1.0f, -1.0f}, {0, 0, 1}},
	};
	vertexBuffer = gl::Buffer(v);

	std::vector<unsigned> ind = { 0, 1, 2 };
	indexBuffer = gl::Buffer(ind);

	pipeline = CreatePipeline();

	resize(GetWindowWidth(), GetWindowHeight());

	return true;
}
//=============================================================================
void NewTest001::OnClose()
{
	vertexBuffer = {};
	pipeline = {};
}
//=============================================================================
void NewTest001::OnUpdate(float deltaTime)
{
}
//=============================================================================
void NewTest001::OnRender()
{
	const gl::SwapChainRenderInfo renderInfo
	{
		.name = "Render Triangle",
		.viewport = {.drawRect{.offset = {0, 0}, .extent = {GetWindowWidth(), GetWindowHeight()}}},
		.colorLoadOp = gl::AttachmentLoadOp::Clear,
		.clearColorValue = {.1f, .5f, .8f, 1.0f},
	};
	gl::BeginSwapChainRendering(renderInfo);
	{
		gl::Cmd::BindGraphicsPipeline(pipeline.value());
		gl::Cmd::BindVertexBuffer(0, vertexBuffer.value(), 0, sizeof(Vertex));
		gl::Cmd::BindIndexBuffer(indexBuffer.value(), gl::IndexType::UInt);
		gl::Cmd::DrawIndexed(3, 1, 0, 0, 0);
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

	DrawFPS();
}
//=============================================================================
void NewTest001::OnResize(uint16_t width, uint16_t height)
{
	resize(width, height);
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