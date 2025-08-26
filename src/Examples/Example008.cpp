#include "stdafx.h"
#include "Example008.h"
//=============================================================================
// Вывод простой сцены с прозрачностью
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
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(aPosition, 1.0);
	vTexCoord = aTexCoord;
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

layout(location = 0) in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D diffuseTexture;

layout(location = 0) out vec4 fragColor;

void main()
{
	fragColor = texture(diffuseTexture, vTexCoord);
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

	Camera camera;

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

	std::optional<gl::Buffer> cubeVB;
	std::optional<gl::Buffer> cubeIB;

	std::optional<gl::Buffer> planeVB;
	std::optional<gl::Buffer> planeIB;

	std::optional<gl::Buffer> windowVB;
	std::optional<gl::Buffer> windowIB;

	std::optional<gl::Buffer> uniformBuffer;

	std::optional<gl::GraphicsPipeline> pipeline;
	std::optional<gl::Texture> texture1;
	std::optional<gl::Texture> texture2;
	std::optional<gl::Texture> texture3;
	std::optional<gl::Sampler> sampler;

	gl::GraphicsPipeline CreatePipeline()
	{
		auto vertexShader = gl::Shader(gl::ShaderType::VertexShader, shaderCodeVertex, "VS");
		auto fragmentShader = gl::Shader(gl::ShaderType::FragmentShader, shaderCodeFragment, "FS");

		gl::ColorBlendState blendState;
		blendState.attachments.push_back({});
		blendState.attachments[0].blendEnable = true;
		blendState.attachments[0].srcColorBlendFactor = gl::BlendFactor::SrcAlpha;
		blendState.attachments[0].dstColorBlendFactor = gl::BlendFactor::OneMinusSrcAlpha;
		blendState.attachments[0].colorBlendOp = gl::BlendOp::Add;
		blendState.attachments[0].srcAlphaBlendFactor = gl::BlendFactor::SrcAlpha;
		blendState.attachments[0].dstAlphaBlendFactor = gl::BlendFactor::OneMinusSrcAlpha;
		blendState.attachments[0].alphaBlendOp = gl::BlendOp::Add;

		return gl::GraphicsPipeline({
			 .name = "Pipeline",
			.vertexShader = &vertexShader,
			.fragmentShader = &fragmentShader,
			.inputAssemblyState = {.topology = gl::PrimitiveTopology::TriangleList },
			.vertexInputState = { inputBindingDescs },
			.rasterizationState = { .cullMode = gl::CullMode::None },
			.depthState = {.depthTestEnable = true },
			.colorBlendState = blendState
			});
	}

	void resize([[maybe_unused]] uint16_t width, [[maybe_unused]] uint16_t height)
	{
	}
}
//=============================================================================
EngineCreateInfo Example008::GetCreateInfo() const
{
	return {};
}
//=============================================================================
bool Example008::OnInit()
{
	std::vector<Vertex> cubeVerts = {
		// Передняя грань (Z = 0.5)
		{{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}},
		{{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}},
		{{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
		{{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}},

		// Правая грань (X = 0.5)
		{{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}},
		{{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
		{{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
		{{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}},

		// Задняя грань (Z = -0.5)
		{{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
		{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
		{{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
		{{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}},

		// Левая грань (X = -0.5)
		{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
		{{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}},
		{{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
		{{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}},

		// Верхняя грань (Y = 0.5)
		{{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}},
		{{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f}},
		{{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
		{{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}},

		// Нижняя грань (Y = -0.5)
		{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
		{{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
		{{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}},
		{{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}},
	};
	cubeVB = gl::Buffer(cubeVerts);

	std::vector<unsigned> cubeIndices = {
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
	cubeIB = gl::Buffer(cubeIndices);

	std::vector<Vertex> planeVerts = {
		{{-5.0f,  0.0f,  5.0f}, {0.0f, 0.0f}},
		{{ 5.0f,  0.0f,  5.0f}, {2.0f, 0.0f}},
		{{ 5.0f,  0.0f, -5.0f}, {2.0f, 2.0f}},
		{{-5.0f,  0.0f, -5.0f}, {0.0f, 2.0f}},
	};
	planeVB = gl::Buffer(planeVerts);

	std::vector<unsigned> planeIndices = {
		0, 2, 1,
		0, 3, 2
	};
	planeIB = gl::Buffer(planeIndices);

	std::vector<Vertex> windowVerts = {
		{{-0.5f, -0.5f,  0.0f}, {0.0f, 0.0f}},
		{{ 0.5f, -0.5f,  0.0f}, {1.0f, 0.0f}},
		{{ 0.5f,  0.5f,  0.0f}, {1.0f, 1.0f}},
		{{-0.5f,  0.5f,  0.0f}, {0.0f, 1.0f}},
	};
	windowVB = gl::Buffer(windowVerts);

	std::vector<unsigned> windowIndices = {
		0, 2, 1,
		0, 3, 2,
	};
	windowIB = gl::Buffer(windowIndices);

	uniformBuffer = gl::Buffer(sizeof(vsUniforms), gl::BufferStorageFlag::DynamicStorage);

	pipeline = CreatePipeline();

	{
		int imgW, imgH, nrChannels;
		auto pixels = stbi_load("ExampleData/textures/metal.png", &imgW, &imgH, &nrChannels, 4);

		const gl::TextureCreateInfo createInfo{
		  .imageType = gl::ImageType::Tex2D,
		  .format = gl::Format::R8G8B8A8_UNORM,
		  .extent = {static_cast<uint32_t>(imgW), static_cast<uint32_t>(imgH), 1},
		  .mipLevels = 1,
		  .arrayLayers = 1,
		  .sampleCount = gl::SampleCount::Samples1,
		};
		texture1 = gl::Texture(createInfo);

		texture1->UpdateImage({
		  .extent = createInfo.extent,
		  .format = gl::UploadFormat::RGBA,
		  .type = gl::UploadType::UBYTE,
		  .pixels = pixels,
			});
		stbi_image_free(pixels);
	}

	{
		int imgW, imgH, nrChannels;
		auto pixels = stbi_load("ExampleData/textures/marble.jpg", &imgW, &imgH, &nrChannels, 4);

		const gl::TextureCreateInfo createInfo{
		  .imageType = gl::ImageType::Tex2D,
		  .format = gl::Format::R8G8B8A8_UNORM,
		  .extent = {static_cast<uint32_t>(imgW), static_cast<uint32_t>(imgH), 1},
		  .mipLevels = 1,
		  .arrayLayers = 1,
		  .sampleCount = gl::SampleCount::Samples1,
		};
		texture2 = gl::Texture(createInfo);

		texture2->UpdateImage({
		  .extent = createInfo.extent,
		  .format = gl::UploadFormat::RGBA,
		  .type = gl::UploadType::UBYTE,
		  .pixels = pixels,
			});
		stbi_image_free(pixels);
	}

	{
		int imgW, imgH, nrChannels;
		auto pixels = stbi_load("ExampleData/textures/transparent_window.png", &imgW, &imgH, &nrChannels, 4);

		const gl::TextureCreateInfo createInfo{
		  .imageType = gl::ImageType::Tex2D,
		  .format = gl::Format::R8G8B8A8_UNORM,
		  .extent = {static_cast<uint32_t>(imgW), static_cast<uint32_t>(imgH), 1},
		  .mipLevels = 1,
		  .arrayLayers = 1,
		  .sampleCount = gl::SampleCount::Samples1,
		};
		texture3 = gl::Texture(createInfo);

		texture3->UpdateImage({
		  .extent = createInfo.extent,
		  .format = gl::UploadFormat::RGBA,
		  .type = gl::UploadType::UBYTE,
		  .pixels = pixels,
			});
		stbi_image_free(pixels);
	}

	gl::SamplerState sampleDesc;
	sampleDesc.minFilter = gl::MinFilter::Linear;
	sampleDesc.magFilter = gl::MagFilter::Linear;
	sampleDesc.addressModeU = gl::AddressMode::Repeat;
	sampleDesc.addressModeV = gl::AddressMode::Repeat;
	sampler = gl::Sampler(sampleDesc);

	camera.SetPosition(glm::vec3(0.0f, 0.0f, -3.0f));

	resize(GetWindowWidth(), GetWindowHeight());

	return true;
}
//=============================================================================
void Example008::OnClose()
{
	cubeVB = {};
	cubeIB = {};
	planeVB = {};
	planeIB = {};
	windowVB = {};
	windowIB = {};
	uniformBuffer = {};
	pipeline = {};
	sampler = {};
	texture1 = {};
	texture2 = {};
	texture3 = {};
}
//=============================================================================
void Example008::OnUpdate([[maybe_unused]] float deltaTime)
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

	uniforms.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	uniforms.viewMatrix = camera.GetViewMatrix();
	uniforms.projectionMatrix = glm::perspective(glm::radians(65.0f), GetWindowAspect(), 0.1f, 100.0f);
}
//=============================================================================
void Example008::OnRender()
{
	const gl::SwapChainRenderInfo renderInfo
	{
		.name = "Render Triangle",
		.viewport = {.drawRect{.offset = {0, 0}, .extent = {GetWindowWidth(), GetWindowHeight()}}},
		.colorLoadOp = gl::AttachmentLoadOp::Clear,
		.clearColorValue = {.1f, .5f, .8f, 1.0f},
		.depthLoadOp = gl::AttachmentLoadOp::Clear,
		.clearDepthValue = 1.0f,
	};
	gl::BeginSwapChainRendering(renderInfo);
	{
		gl::Cmd::BindGraphicsPipeline(pipeline.value());

		// плоскость
		{
			uniforms.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f));
			uniformBuffer->UpdateData(uniforms);
			gl::Cmd::BindSampledImage(0, texture1.value(), sampler.value());
			gl::Cmd::BindUniformBuffer(0, uniformBuffer.value());
			gl::Cmd::BindVertexBuffer(0, planeVB.value(), 0, sizeof(Vertex));
			gl::Cmd::BindIndexBuffer(planeIB.value(), gl::IndexType::UInt);
			gl::Cmd::DrawIndexed(6, 1, 0, 0, 0);
		}

		// куб 1
		{
			uniforms.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 1.0f));
			uniformBuffer->UpdateData(uniforms);
			gl::Cmd::BindSampledImage(0, texture2.value(), sampler.value());
			gl::Cmd::BindUniformBuffer(0, uniformBuffer.value());
			gl::Cmd::BindVertexBuffer(0, cubeVB.value(), 0, sizeof(Vertex));
			gl::Cmd::BindIndexBuffer(cubeIB.value(), gl::IndexType::UInt);
			gl::Cmd::DrawIndexed(36, 1, 0, 0, 0);
		}
		// куб 2
		{
			uniforms.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));
			uniformBuffer->UpdateData(uniforms);
			gl::Cmd::BindSampledImage(0, texture2.value(), sampler.value());
			gl::Cmd::BindUniformBuffer(0, uniformBuffer.value());
			gl::Cmd::BindVertexBuffer(0, cubeVB.value(), 0, sizeof(Vertex));
			gl::Cmd::BindIndexBuffer(cubeIB.value(), gl::IndexType::UInt);
			gl::Cmd::DrawIndexed(36, 1, 0, 0, 0);
		}

		// окна
		{
			glm::vec3 vegetation[5];
			vegetation[0] = glm::vec3(-1.5f, 0.0f, 0.48f);
			vegetation[1] = glm::vec3(1.5f, 0.0f, -0.51f);
			vegetation[2] = glm::vec3(0.0f, 0.0f, -0.7f);
			vegetation[3] = glm::vec3(-0.3f, 0.0f, 2.3f);
			vegetation[4] = glm::vec3(0.5f, 0.0f, 0.6f);

			glm::vec3 position = camera.Position;

			// simple bubble sort algorithm to sort vegetation from furthest to nearest
			for (int i = 1; i < 5; ++i)
			{
				for (int j = i - 1; j >= 0; --j)
				{
					glm::vec3 translate0 = vegetation[j] - position;
					glm::vec3 translate1 = vegetation[j + 1] - position;

					if (glm::length(translate0) < glm::length(translate1))
					{
						glm::vec3  temp = vegetation[j];
						vegetation[j] = vegetation[j + 1];
						vegetation[j + 1] = temp;
						break;
					}
				}
			}

			for (size_t i = 0; i < 5; i++)
			{
				uniforms.modelMatrix = glm::translate(glm::mat4(1.0f), vegetation[i]);
				uniformBuffer->UpdateData(uniforms);
				gl::Cmd::BindSampledImage(0, texture3.value(), sampler.value());
				gl::Cmd::BindUniformBuffer(0, uniformBuffer.value());
				gl::Cmd::BindVertexBuffer(0, windowVB.value(), 0, sizeof(Vertex));
				gl::Cmd::BindIndexBuffer(windowIB.value(), gl::IndexType::UInt);
				gl::Cmd::DrawIndexed(6, 1, 0, 0, 0);
			}
		}
	}
	gl::EndRendering();
}
//=============================================================================
void Example008::OnImGuiDraw()
{
	DrawFPS();
}
//=============================================================================
void Example008::OnResize(uint16_t width, uint16_t height)
{
	resize(width, height);
}
//=============================================================================
void Example008::OnMouseButton([[maybe_unused]] int button, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================
void Example008::OnMousePos([[maybe_unused]] double x, [[maybe_unused]] double y)
{
}
//=============================================================================
void Example008::OnScroll([[maybe_unused]] double dx, [[maybe_unused]] double dy)
{
}
//=============================================================================
void Example008::OnKey([[maybe_unused]] int key, [[maybe_unused]] int scanCode, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================