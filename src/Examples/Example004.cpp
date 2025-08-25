#include "stdafx.h"
#include "Example004.h"
//=============================================================================
// Вывод кубов на сцену и движение по ней с помощью камеры
// - вывод кубов
// - Camera
//=============================================================================
namespace
{
	const char* shaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

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
		glm::vec3 normal;
		glm::vec2 uv;
	};

	Camera camera;

	constexpr std::array<gl::VertexInputBindingDescription, 3> inputBindingDescs{
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
			.offset = offsetof(Vertex, normal),
		},
		gl::VertexInputBindingDescription{
			.location = 2,
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

	void resize([[maybe_unused]] uint16_t width, [[maybe_unused]] uint16_t height)
	{
	}
}
//=============================================================================
EngineCreateInfo Example004::GetCreateInfo() const
{
	return {};
}
//=============================================================================
bool Example004::OnInit()
{
	std::vector<Vertex> v = {
		//// Передняя грань (Z = 0.5)
		//{{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}},
		//{{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}},
		//{{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
		//{{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}},

		//// Правая грань (X = 0.5)
		//{{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}},
		//{{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
		//{{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
		//{{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}},

		//// Задняя грань (Z = -0.5)
		//{{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
		//{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
		//{{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
		//{{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}},

		//// Левая грань (X = -0.5)
		//{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
		//{{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}},
		//{{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
		//{{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}},

		//// Верхняя грань (Y = 0.5)
		//{{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}},
		//{{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f}},
		//{{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
		//{{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}},

		//// Нижняя грань (Y = -0.5)
		//{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
		//{{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
		//{{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}},
		//{{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}},

		// Передняя грань (Z = 0.5) — нормаль: (0, 0, 1)
		{{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},
		{{ 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 0.0f}},
		{{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}},
		{{-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f}},

		// Правая грань (X = 0.5) — нормаль: (1, 0, 0)
		{{ 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
		{{ 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
		{{ 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
		{{ 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},

		// Задняя грань (Z = -0.5) — нормаль: (0, 0, -1)
		{{ 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},
		{{-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}},
		{{-0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}},
		{{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}},

		// Левая грань (X = -0.5) — нормаль: (-1, 0, 0)
		{{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
		{{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
		{{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
		{{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},

		// Верхняя грань (Y = 0.5) — нормаль: (0, 1, 0)
		{{-0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}},
		{{ 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}},
		{{ 0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f}},
		{{-0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 1.0f}},

		// Нижняя грань (Y = -0.5) — нормаль: (0, -1, 0)
		{{-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},
		{{ 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}},
		{{ 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 1.0f}},
		{{-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}},
	};
	vertexBuffer = gl::Buffer(v);

	std::vector<unsigned> ind = { 
		// Передняя грань
		0, 2, 1,
		0, 3, 2,

		// Правая грань
		4, 6, 5,
		4, 7, 6,

		// Задняя грань
		8,  10, 9,
		8,  11, 10,

		// Левая грань
		12, 14, 13,
		12, 15, 14,

		// Верхняя грань
		16, 18, 17,
		16, 19, 18,

		// Нижняя грань
		20, 22, 21,
		20, 23, 22
	
	};
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

	camera.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

	resize(GetWindowWidth(), GetWindowHeight());

	return true;
}
//=============================================================================
void Example004::OnClose()
{
	vertexBuffer = {};
	indexBuffer = {};
	uniformBuffer = {};
	pipeline = {};
	sampler = {};
	texture = {};
}
//=============================================================================
void Example004::OnUpdate([[maybe_unused]] float deltaTime)
{
	if (Input::IsKeyDown(GLFW_KEY_W)) camera.ProcessKeyboard(CameraForward, deltaTime);
	if (Input::IsKeyDown(GLFW_KEY_S)) camera.ProcessKeyboard(CameraBackward, deltaTime);
	if (Input::IsKeyDown(GLFW_KEY_A)) camera.ProcessKeyboard(CameraLeft, deltaTime);
	if (Input::IsKeyDown(GLFW_KEY_D)) camera.ProcessKeyboard(CameraRight, deltaTime);

	if (Input::IsMouseDown(GLFW_MOUSE_BUTTON_RIGHT))
	{
		Input::SetCursorVisible(false);
		camera.ProcessMouseMovement(Input::GetScreenOffset().x, Input::GetScreenOffset().y);
	}
	else if (Input::IsMouseReleased(GLFW_MOUSE_BUTTON_RIGHT))
	{
		Input::SetCursorVisible(true);
	}

	uniforms.modelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	uniforms.viewMatrix = camera.GetViewMatrix();
	uniforms.projectionMatrix = glm::perspective(glm::radians(65.0f), GetWindowAspect(), 0.1f, 100.0f);
	uniformBuffer->UpdateData(uniforms);
}
//=============================================================================
void Example004::OnRender()
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
		gl::Cmd::BindUniformBuffer(0, uniformBuffer.value());
		gl::Cmd::BindSampledImage(0, texture.value(), sampler.value());
		gl::Cmd::DrawIndexed(36, 1, 0, 0, 0);
	}
	gl::EndRendering();
}
//=============================================================================
void Example004::OnImGuiDraw()
{
	DrawFPS();
}
//=============================================================================
void Example004::OnResize(uint16_t width, uint16_t height)
{
	resize(width, height);
}
//=============================================================================
void Example004::OnMouseButton([[maybe_unused]] int button, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================
void Example004::OnMousePos([[maybe_unused]] double x, [[maybe_unused]] double y)
{
}
//=============================================================================
void Example004::OnScroll([[maybe_unused]] double dx, [[maybe_unused]] double dy)
{
}
//=============================================================================
void Example004::OnKey([[maybe_unused]] int key, [[maybe_unused]] int scanCode, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================