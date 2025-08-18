#include "stdafx.h"
#include "NewTest003.h"

//=============================================================================
// дополнительные фичи при выводе треугольника
// - один вершинный буфер
// - включение Z буфера
// - рендер в текстуру и вывод текстуры на экран
// - вывод двух треугольников с Z позицией
//=============================================================================
namespace
{
	const char* shaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec3 a_color;

layout(location = 0) out vec3 v_color;

layout(binding = 0) uniform Uniforms { float posZ; };

void main()
{
	v_color = a_color;
	gl_Position = vec4(a_pos.xy, posZ, 1.0);
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

	std::optional<gl::Buffer> vertexBuffer1;
	std::optional<gl::Buffer> vertexBuffer2;

	std::optional<gl::TypedBuffer<float>> uniformBuffer1;
	std::optional<gl::TypedBuffer<float>> uniformBuffer2;
	std::optional<gl::GraphicsPipeline> pipeline;
	std::optional<gl::Texture> msColorTex;
	std::optional<gl::Texture> gDepth;

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
			.depthState = {.depthTestEnable = true},
			});
	}

	void resize(uint16_t width, uint16_t height)
	{
		// размер уменьшить на 8
		msColorTex = gl::CreateTexture2D({ width / 8u, height / 8u }, gl::Format::R8G8B8A8_SRGB, "gAlbedo");

		gDepth = gl::CreateTexture2D({ width / 8u, height / 8u }, gl::Format::D32_FLOAT, "gDepth");
	}
}
//=============================================================================
EngineCreateInfo NewTest003::GetCreateInfo() const
{
	return {};
}
//=============================================================================
bool NewTest003::OnInit()
{
	std::vector<Vertex> v = {
		{{  0.0f,  0.4f}, {1, 0, 0}},
		{{ -1.0f, -1.0f}, {0, 1, 0}},
		{{  1.0f, -1.0f}, {0, 0, 1}},
	};
	vertexBuffer1 = gl::Buffer(std::span(v));

	std::vector<Vertex> v2 = {
	{{  0.0f,  1.0f}, {0, 1, 0}},
	{{ -0.7f, -0.4f}, {0, 1, 1}},
	{{  0.7f, -0.4f}, {1, 0, 1}},
	};
	vertexBuffer2 = gl::Buffer(std::span(v2));

	uniformBuffer1 = gl::TypedBuffer<float>(gl::BufferStorageFlag::DynamicStorage);
	uniformBuffer2 = gl::TypedBuffer<float>(gl::BufferStorageFlag::DynamicStorage);

	pipeline = CreatePipeline();


	resize(GetWindowWidth(), GetWindowHeight());

	return true;
}
//=============================================================================
void NewTest003::OnClose()
{
	vertexBuffer1 = {};
	vertexBuffer2 = {};
	uniformBuffer1 = {};
	uniformBuffer1 = {};
	pipeline = {};
	msColorTex = {};
}
//=============================================================================
void NewTest003::OnUpdate(float deltaTime)
{
	float posZ = 0.0f;
	uniformBuffer1->UpdateData(posZ);
	posZ = 0.5f;
	uniformBuffer2->UpdateData(posZ);
}
//=============================================================================
void NewTest003::OnRender()
{
	auto attachment = gl::RenderColorAttachment{
		.texture = msColorTex.value(),
		.loadOp = gl::AttachmentLoadOp::Clear,
		.clearValue = {.1f, .5f, .8f, 1.0f},
	};

	auto gDepthAttachment = gl::RenderDepthStencilAttachment{
	  .texture = gDepth.value(),
	  .loadOp = gl::AttachmentLoadOp::Clear,
	  .clearValue = {.depth = 1.0f},
	};

	gl::BeginRendering({ 
		.colorAttachments = {&attachment, 1},
		.depthAttachment = gDepthAttachment
		});
	{
		gl::Cmd::BindGraphicsPipeline(pipeline.value());
		gl::Cmd::BindVertexBuffer(0, vertexBuffer1.value(), 0, sizeof(Vertex));
		gl::Cmd::BindUniformBuffer(0, uniformBuffer1.value());
		gl::Cmd::Draw(3, 1, 0, 0);

		gl::Cmd::BindGraphicsPipeline(pipeline.value());
		gl::Cmd::BindVertexBuffer(0, vertexBuffer2.value(), 0, sizeof(Vertex));
		gl::Cmd::BindUniformBuffer(0, uniformBuffer2.value());
		gl::Cmd::Draw(3, 1, 0, 0);
	}
	gl::EndRendering();

	gl::BlitTextureToSwapChain(*msColorTex,
		{},
		{},
		msColorTex->Extent(),
		{ GetWindowWidth(), GetWindowHeight(), 1 },
		gl::MagFilter::Nearest);
}
//=============================================================================
void NewTest003::OnImGuiDraw()
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
void NewTest003::OnResize(uint16_t width, uint16_t height)
{
	resize(width, height);
}
//=============================================================================
void NewTest003::OnMouseButton(int button, int action, int mods)
{
}
//=============================================================================
void NewTest003::OnMousePos(double x, double y)
{
}
//=============================================================================
void NewTest003::OnScroll(double dx, double dy)
{
}
//=============================================================================
void NewTest003::OnKey(int key, int scanCode, int action, int mods)
{
}
//=============================================================================