#include "stdafx.h"
#include "Example003.h"
//=============================================================================
// Вывод прямоугольника с текстурой на основную поверхность с матрицами трансформации
// - юниформ буфер и MVP матрицы
//=============================================================================
namespace
{
	const char* shaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;

layout(binding = 0, std140) uniform vsUniforms {
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
};

layout(location = 0) out vec2 vTexCoord;

void main()
{
	vTexCoord = aTexCoord;
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(aPosition, 1.0);
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

layout(location = 0) in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D diffuseTex;

layout(location = 0) out vec4 fragColor;

void main()
{
	fragColor = texture(diffuseTex, vTexCoord);
}
)";

	struct vsUniforms final
	{
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
	};
	vsUniforms uniforms;

	struct Vertex final
	{
		glm::vec3 pos;
		glm::vec2 uv;
	};

	constexpr std::array<gl::VertexInputBindingDescription, 2> inputBindingDescs{
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

	std::optional<gl::Buffer> vertexBuffer;
	std::optional<gl::Buffer> indexBuffer;
	std::optional<gl::Buffer> uniformBuffer;
	std::optional<gl::GraphicsPipeline> pipeline;
	std::optional<gl::Texture> texture;
	std::optional<gl::Sampler> sampler;

	gl::GraphicsPipeline CreatePipeline()
	{
		auto vertexShader = gl::Shader(gl::ShaderType::VertexShader, shaderCodeVertex, "VS");
		auto fragmentShader = gl::Shader(gl::ShaderType::FragmentShader, shaderCodeFragment, "FS");

		return gl::GraphicsPipeline({
			 .name = "Pipeline",
			.vertexShader = &vertexShader,
			.fragmentShader = &fragmentShader,
			.inputAssemblyState = {.topology = gl::PrimitiveTopology::TriangleList},
			.vertexInputState = {inputBindingDescs},
			});
	}
}
//=============================================================================
EngineCreateInfo Example003::GetCreateInfo() const
{
	return {};
}
//=============================================================================
bool Example003::OnInit()
{
	std::vector<Vertex> v = {
		{{ -0.8f,  0.8f, 0.0f}, {0.0f, 0.0f}},
		{{ -0.8f, -0.8f, 0.0f}, {0.0f, 1.0f}},
		{{  0.8f, -0.8f, 0.0f}, {1.0f, 1.0f}},
		{{  0.8f,  0.8f, 0.0f}, {1.0f, 0.0f}},
	};
	vertexBuffer = gl::Buffer(v);

	std::vector<unsigned> ind = { 0, 1, 2, 2, 3, 0 };
	indexBuffer = gl::Buffer(ind);

	uniformBuffer = gl::Buffer(sizeof(vsUniforms), gl::BufferStorageFlag::DynamicStorage);

	pipeline = CreatePipeline();

	{
		int imgW, imgH, nrChannels;
		auto pixels = stbi_load("CoreData/textures/temp.png", &imgW, &imgH, &nrChannels, 4);

		const gl::TextureCreateInfo createInfo{
		  .imageType = gl::ImageType::Tex2D,
		  .format = gl::Format::R8G8B8A8_UNORM,
		  .extent = {static_cast<uint32_t>(imgW), static_cast<uint32_t>(imgH), 1},
		  .mipLevels = 1,
		  .arrayLayers = 1,
		  .sampleCount = gl::SampleCount::Samples1,
		};
		texture = gl::Texture(createInfo);

		texture->UpdateImage({
		  .extent = createInfo.extent,
		  .format = gl::UploadFormat::RGBA,
		  .type = gl::UploadType::UBYTE,
		  .pixels = pixels,
			});
		stbi_image_free(pixels);
	}

	gl::SamplerState sampleDesc;
	sampleDesc.minFilter = gl::MinFilter::Nearest;
	sampleDesc.magFilter = gl::MagFilter::Nearest;
	sampleDesc.addressModeU = gl::AddressMode::Repeat;
	sampleDesc.addressModeV = gl::AddressMode::Repeat;
	sampler = gl::Sampler(sampleDesc);

	return true;
}
//=============================================================================
void Example003::OnClose()
{
	vertexBuffer = {};
	indexBuffer = {};
	uniformBuffer = {};
	pipeline = {};
	sampler = {};
	texture = {};
}
//=============================================================================
void Example003::OnUpdate([[maybe_unused]] float deltaTime)
{
	uniforms.modelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	uniforms.viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 3.0f));
	uniforms.projectionMatrix = glm::perspective(glm::radians(65.0f), GetWindowAspect(), 0.1f, 100.0f);
	uniformBuffer->UpdateData(uniforms);
}
//=============================================================================
void Example003::OnRender()
{
	const gl::SwapChainRenderInfo renderInfo
	{
		.name = "Render",
		.viewport = {.drawRect{.offset = {0, 0}, .extent = {GetWindowWidth(), GetWindowHeight()}}},
		.colorLoadOp = gl::AttachmentLoadOp::Clear,
		.clearColorValue = {.1f, .5f, .8f, 1.0f},
	};
	gl::BeginSwapChainRendering(renderInfo);
	{
		gl::Cmd::BindGraphicsPipeline(pipeline.value());
		gl::Cmd::BindVertexBuffer(0, vertexBuffer.value(), 0, sizeof(Vertex));
		gl::Cmd::BindIndexBuffer(indexBuffer.value(), gl::IndexType::UInt);
		gl::Cmd::BindUniformBuffer(0, uniformBuffer.value());
		gl::Cmd::BindSampledImage(0, texture.value(), sampler.value());
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