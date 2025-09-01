#include "stdafx.h"
#include "Example002.h"
//=============================================================================
namespace
{
	const char* shaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec2 vertexTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main()
{
	fragColor    = vertexColor;
	fragTexCoord = vertexTexCoord;
	gl_Position  = vec4(vertexPosition, 1.0);
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 0) uniform sampler2D diffuseTex;

layout(location = 0) out vec4 outputColor;

void main()
{
	outputColor = texture(diffuseTex, fragTexCoord) * vec4(fragColor, 1.0);
}
)";

	struct Vertex final
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 uv;
	};

	constexpr std::array<gl::VertexInputBindingDescription, 3> inputBindingDesc{
		gl::VertexInputBindingDescription{
			.location = 0,
			.binding = 0,
			.format = gl::Format::R32G32B32_FLOAT,
			.offset = offsetof(Vertex, pos),
		},
		gl::VertexInputBindingDescription{
			.location = 1,
			.binding = 0,
			.format = gl::Format::R32G32B32_FLOAT,
			.offset = offsetof(Vertex, color),
		},
		gl::VertexInputBindingDescription{
			.location = 2,
			.binding = 0,
			.format = gl::Format::R32G32_FLOAT,
			.offset = offsetof(Vertex, uv),
		},
	};
}
//=============================================================================
EngineCreateInfo Example002::GetCreateInfo() const
{
	EngineCreateInfo createInfo{};
	return createInfo;
}
//=============================================================================
bool Example002::OnInit()
{
	//-------------------------------------------------------------------------
	// create vertex buffer
	std::vector<Vertex> v = {
		{{ -0.8f,  0.8f, 0.0f}, {1, 0, 0}, {0.0f, 0.0f}},
		{{ -0.8f, -0.8f, 0.0f}, {0, 1, 0}, {0.0f, 1.0f}},
		{{  0.8f, -0.8f, 0.0f}, {0, 0, 1}, {1.0f, 1.0f}},
		{{  0.8f,  0.8f, 0.0f}, {1, 1, 0}, {1.0f, 0.0f}},
	};
	m_vertexBuffer = gl::Buffer(v);

	//-------------------------------------------------------------------------
	// create index buffer
	std::vector<unsigned> ind = { 0, 1, 2, 2, 3, 0};
	m_indexBuffer = gl::Buffer(ind);

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
	// load texture
	{
		int imgW, imgH, nrChannels;
		auto pixels = stbi_load("CoreData/textures/temp.png", &imgW, &imgH, &nrChannels, 4);
		const Extent3D extent = { static_cast<uint32_t>(imgW), static_cast<uint32_t>(imgH), 1 };

		m_texture = gl::Texture({
			.imageType   = gl::ImageType::Tex2D,
			.format      = gl::Format::R8G8B8A8_UNORM,
			.extent      = extent,
			.mipLevels   = 1u,
			.arrayLayers = 1u,
			.sampleCount = gl::SampleCount::Samples1,
		});

		m_texture->UpdateImage({
			.extent = extent,
			.format = gl::UploadFormat::RGBA,
			.type   = gl::UploadType::UBYTE,
			.pixels = pixels,
		});
		stbi_image_free(pixels);
	}

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
void Example002::OnClose()
{
	m_vertexBuffer = {};
	m_indexBuffer = {};
	m_pipeline = {};
	m_sampler = {};
	m_texture = {};
}
//=============================================================================
void Example002::OnUpdate([[maybe_unused]] float deltaTime)
{
}
//=============================================================================
void Example002::OnRender()
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
		gl::Cmd::BindSampledImage(0, *m_texture, *m_sampler);
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
void Example002::OnResize([[maybe_unused]] uint16_t width, [[maybe_unused]] uint16_t height)
{
}
//=============================================================================
void Example002::OnMouseButton([[maybe_unused]] int button, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================
void Example002::OnMousePos([[maybe_unused]] double x, [[maybe_unused]] double y)
{
}
//=============================================================================
void Example002::OnScroll([[maybe_unused]] double dx, [[maybe_unused]] double dy)
{
}
//=============================================================================
void Example002::OnKey([[maybe_unused]] int key, [[maybe_unused]] int scanCode, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================