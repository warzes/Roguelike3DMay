#include "stdafx.h"
#include "Example002.h"
//=============================================================================
// Вывод прямоугольника на основную поверхность
// - текстура
//=============================================================================
namespace
{
	const char* shaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 a_color;
layout(location = 1) in vec3 a_color;

layout(location = 0) out vec3 v_color;

void main()
{
	v_color = a_color;
	gl_Position = vec4(a_pos.xy, 0.0, 1.0);
}

in vec3 position;
in vec3 aColor;
in vec2 aTexCoord;
  
out vec3 ourColor;
out vec2 TexCoord;

void main() {
    gl_Position = vec4(position, 1.0);
    ourColor = aColor;
    TexCoord = aTexCoord;
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
EngineCreateInfo Example002::GetCreateInfo() const
{
	return {};
}
//=============================================================================
bool Example002::OnInit()
{
	std::vector<Vertex> v = {
		{{ -0.8f,  0.8f}, {1, 0, 0}},
		{{ -0.8f, -0.8f}, {0, 1, 0}},
		{{  0.8f, -0.8f}, {0, 0, 1}},
		{{  0.8f,  0.8f}, {1, 1, 0}},
	};
	vertexBuffer = gl::Buffer(v);

	std::vector<unsigned> ind = { 0, 1, 2, 2, 3, 0};
	indexBuffer = gl::Buffer(ind);

	pipeline = CreatePipeline();

	resize(GetWindowWidth(), GetWindowHeight());

	return true;
}
//=============================================================================
void Example002::OnClose()
{
	vertexBuffer = {};
	pipeline = {};
}
//=============================================================================
void Example002::OnUpdate(float deltaTime)
{
}
//=============================================================================
void Example002::OnRender()
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
		gl::Cmd::DrawIndexed(6, 1, 0, 0, 0);
	}
	gl::EndRendering();
}
//=============================================================================
void Example002::OnImGuiDraw()
{
	DrawFPS();
}
//=============================================================================
void Example002::OnResize(uint16_t width, uint16_t height)
{
	resize(width, height);
}
//=============================================================================
void Example002::OnMouseButton(int button, int action, int mods)
{
}
//=============================================================================
void Example002::OnMousePos(double x, double y)
{
}
//=============================================================================
void Example002::OnScroll(double dx, double dy)
{
}
//=============================================================================
void Example002::OnKey(int key, int scanCode, int action, int mods)
{
}
//=============================================================================