#include "stdafx.h"
#include "Example001.h"
//=============================================================================
namespace
{
	const char* shaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec2 vertexPosition;
layout(location = 1) in vec3 vertexColor;

out vec3 fragColor;

void main()
{
	fragColor   = vertexColor;
	gl_Position = vec4(vertexPosition.xy, 0.0, 1.0);
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

in vec3 fragColor;

layout(location = 0) out vec4 outputColor;

void main()
{
	outputColor = vec4(fragColor, 1.0);
}
)";

	struct Vertex final
	{
		glm::vec2 pos;
		glm::vec3 color;
	};
	constexpr std::array<gl::VertexInputBindingDescription, 2> inputBindingDesc{
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
}
//=============================================================================
EngineCreateInfo Example001::GetCreateInfo() const
{
	EngineCreateInfo createInfo{};
	return createInfo;
}
//=============================================================================
bool Example001::OnInit()
{
	//-------------------------------------------------------------------------
	// create vertex buffer
	std::vector<Vertex> v = {
		{{  0.0f,  0.4f}, {1, 0, 0}},
		{{ -1.0f, -1.0f}, {0, 1, 0}},
		{{  1.0f, -1.0f}, {0, 0, 1}},
	};
	m_vertexBuffer = gl::Buffer(v);

	//-------------------------------------------------------------------------
	// create index buffer
	std::vector<unsigned> ind = { 0, 1, 2 };
	m_indexBuffer = gl::Buffer(ind);

	//-------------------------------------------------------------------------
	// create pipeline
	auto vertexShader   = gl::Shader(gl::ShaderType::VertexShader, shaderCodeVertex, "Triangle VS");
	auto fragmentShader = gl::Shader(gl::ShaderType::FragmentShader, shaderCodeFragment, "Triangle FS");

	m_pipeline = gl::GraphicsPipeline({ 
		.name               = "Triangle Pipeline",
		.vertexShader       = &vertexShader,
		.fragmentShader     = &fragmentShader,
		.inputAssemblyState = {.topology = gl::PrimitiveTopology::TriangleList },
		.vertexInputState   = { inputBindingDesc },
	});

	return true;
}
//=============================================================================
void Example001::OnClose()
{
	m_vertexBuffer = {};
	m_indexBuffer = {};
	m_pipeline = {};
}
//=============================================================================
void Example001::OnUpdate([[maybe_unused]] float deltaTime)
{
}
//=============================================================================
void Example001::OnRender()
{
	const gl::SwapChainRenderInfo renderInfo {
		.name = "Render Triangle",
		.viewport = {.drawRect{.offset = {0, 0}, .extent = {GetWindowWidth(), GetWindowHeight()}}},
		.colorLoadOp = gl::AttachmentLoadOp::Clear,
		.clearColorValue = {0.1f, 0.5f, 0.8f, 1.0f},
	};

	gl::BeginSwapChainRendering(renderInfo);
	{
		gl::Cmd::BindGraphicsPipeline(*m_pipeline);
		gl::Cmd::BindVertexBuffer(0, *m_vertexBuffer, 0, sizeof(Vertex));
		gl::Cmd::BindIndexBuffer(*m_indexBuffer, gl::IndexType::UInt);
		gl::Cmd::DrawIndexed(3, 1, 0, 0, 0);
	}
	gl::EndRendering();
}
//=============================================================================
void Example001::OnImGuiDraw()
{
	ImGui::Begin("Simple");

	ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Vendor: %s", (char*)glGetString(GL_VENDOR));
	ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Version: %s", (char*)glGetString(GL_VERSION));
	ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Renderer: %s", (char*)glGetString(GL_RENDERER));

	ImGui::End();

	DrawFPS();
}
//=============================================================================
void Example001::OnResize([[maybe_unused]] uint16_t width, [[maybe_unused]] uint16_t height)
{
}
//=============================================================================
void Example001::OnMouseButton([[maybe_unused]] int button, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================
void Example001::OnMousePos([[maybe_unused]] double x, [[maybe_unused]] double y)
{
}
//=============================================================================
void Example001::OnScroll([[maybe_unused]] double dx, [[maybe_unused]] double dy)
{
}
//=============================================================================
void Example001::OnKey([[maybe_unused]] int key, [[maybe_unused]] int scanCode, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================