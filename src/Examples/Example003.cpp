#include "stdafx.h"
#include "Example003.h"
//=============================================================================
namespace
{
	const char* shaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoord;

layout(binding = 0, std140) uniform MatrixBlock {
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
};

layout(location = 0) out vec2 fragTexCoord;

void main()
{
	fragTexCoord = vertexTexCoord;
	gl_Position  = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

layout(location = 0) in vec2 fragTexCoord;

layout(binding = 0) uniform sampler2D diffuseTex;

layout(location = 0) out vec4 outputColor;

void main()
{
	outputColor = texture(diffuseTex, fragTexCoord);
}
)";

	struct alignas(16) vsUniforms final
	{
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
	};
	vsUniforms uniformsData;

	struct Vertex final
	{
		glm::vec3 pos;
		glm::vec2 uv;
	};

	constexpr std::array<gl::VertexInputBindingDescription, 2> inputBindingDesc{
		gl::VertexInputBindingDescription{
			.location = 0,
			.binding = 0,
			.format = gl::Format::R32G32B32_FLOAT,
			.offset = offsetof(Vertex, pos),
		},
		gl::VertexInputBindingDescription{
			.location = 1,
			.binding = 0,
			.format = gl::Format::R32G32_FLOAT,
			.offset = offsetof(Vertex, uv),
		}
	};
}
//=============================================================================
EngineCreateInfo Example003::GetCreateInfo() const
{
	EngineCreateInfo createInfo{};
	return createInfo;
}
//=============================================================================
bool Example003::OnInit()
{
	//-------------------------------------------------------------------------
	// create vertex buffer
	std::vector<Vertex> v = {
		{{ -0.8f,  0.8f, 0.0f}, {0.0f, 0.0f}},
		{{ -0.8f, -0.8f, 0.0f}, {0.0f, 1.0f}},
		{{  0.8f, -0.8f, 0.0f}, {1.0f, 1.0f}},
		{{  0.8f,  0.8f, 0.0f}, {1.0f, 0.0f}},
	};
	m_vertexBuffer = gl::Buffer(v);

	//-------------------------------------------------------------------------
	// create index buffer
	std::vector<unsigned> ind = { 0, 1, 2, 2, 3, 0 };
	m_indexBuffer = gl::Buffer(ind);

	//-------------------------------------------------------------------------
	// create uniform buffer
	m_uniformBuffer = gl::Buffer(sizeof(vsUniforms), gl::BufferStorageFlag::DynamicStorage);

	//-------------------------------------------------------------------------
	// create pipeline
	auto vertexShader   = gl::Shader(gl::ShaderType::VertexShader, shaderCodeVertex, "VS");
	auto fragmentShader = gl::Shader(gl::ShaderType::FragmentShader, shaderCodeFragment, "FS");

	m_pipeline = gl::GraphicsPipeline({
		.name               = "Pipeline",
		.vertexShader       = &vertexShader,
		.fragmentShader     = &fragmentShader,
		.inputAssemblyState = {.topology = gl::PrimitiveTopology::TriangleList},
		.vertexInputState   = {inputBindingDesc},
	});

	//-------------------------------------------------------------------------
	// load srgb texture
	m_texture = TextureManager::GetTexture("CoreData/textures/temp.png", gl::ColorSpace::sRGB);

	//-------------------------------------------------------------------------
	// create Sampler
	m_sampler = gl::Sampler({
		.minFilter    = gl::MinFilter::Nearest,
		.magFilter    = gl::MagFilter::Nearest,
		.addressModeU = gl::AddressMode::Repeat,
		.addressModeV = gl::AddressMode::Repeat,
	});

	return true;
}
//=============================================================================
void Example003::OnClose()
{
	m_vertexBuffer = {};
	m_indexBuffer = {};
	m_uniformBuffer = {};
	m_pipeline = {};
	m_sampler = {};
	m_texture = nullptr;
}
//=============================================================================
void Example003::OnUpdate([[maybe_unused]] float deltaTime)
{
	uniformsData.modelMatrix      = glm::rotate(glm::mat4(1.0f), glm::radians(65.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	uniformsData.viewMatrix       = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.4f));
	uniformsData.projectionMatrix = glm::perspective(glm::radians(65.0f), GetWindowAspect(), 0.1f, 100.0f);

	// Update UBO data
	m_uniformBuffer->UpdateData(uniformsData);
}
//=============================================================================
void Example003::OnRender()
{
	const gl::SwapChainRenderInfo renderInfo {
		.name = "Render",
		.viewport = {.drawRect{.offset = {0, 0}, .extent = {GetWindowWidth(), GetWindowHeight()}}},
		.colorLoadOp = gl::AttachmentLoadOp::Clear,
		.clearColorValue = { 0.1f, 0.5f, 0.8f, 1.0f },
	};
	gl::BeginSwapChainRendering(renderInfo);
	{
		gl::Cmd::BindGraphicsPipeline(*m_pipeline);
		gl::Cmd::BindVertexBuffer(0, *m_vertexBuffer, 0, sizeof(Vertex));
		gl::Cmd::BindIndexBuffer(*m_indexBuffer, gl::IndexType::UInt);
		gl::Cmd::BindUniformBuffer(0, *m_uniformBuffer);
		gl::Cmd::BindSampledImage(0, *m_texture, *m_sampler);
		gl::Cmd::DrawIndexed(6, 1, 0, 0, 0);
	}
	gl::EndRendering();
}
//=============================================================================
void Example003::OnImGuiDraw()
{
	DrawFPS();
}
//=============================================================================
void Example003::OnResize([[maybe_unused]] uint16_t width, [[maybe_unused]] uint16_t height)
{
}
//=============================================================================
void Example003::OnMouseButton([[maybe_unused]] int button, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================
void Example003::OnMousePos([[maybe_unused]] double x, [[maybe_unused]] double y)
{
}
//=============================================================================
void Example003::OnScroll([[maybe_unused]] double dx, [[maybe_unused]] double dy)
{
}
//=============================================================================
void Example003::OnKey([[maybe_unused]] int key, [[maybe_unused]] int scanCode, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{


}
//=============================================================================