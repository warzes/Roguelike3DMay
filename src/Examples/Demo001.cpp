#include "stdafx.h"
#include "Demo001.h"
//=============================================================================
// Вывод простой сцены с прозрачностью
//=============================================================================
namespace
{
	const char* shaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec2 aTexCoord;
layout(location = 4) in vec3 aTangent;

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

	Camera camera;

	Model plane;
	Model box;

	std::optional<gl::Buffer> uniformBuffer;

	std::optional<gl::GraphicsPipeline> pipeline;
	std::optional<gl::Texture> texture1;
	std::optional<gl::Texture> texture2;
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
			.vertexInputState = { MeshVertexInputBindingDescs },
			.depthState = {.depthTestEnable = true },
			.colorBlendState = blendState
			});
	}

	void resize([[maybe_unused]] uint16_t width, [[maybe_unused]] uint16_t height)
	{
	}
}
//=============================================================================
EngineCreateInfo Demo001::GetCreateInfo() const
{
	return {};
}
//=============================================================================
bool Demo001::OnInit()
{
	box.Create(GeometryGenerator::CreateBox());
	plane.Create(GeometryGenerator::CreatePlane(10.0f, 10.0f, 10.0f, 10.0f));

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
void Demo001::OnClose()
{
	box.Free();
	plane.Free();
	uniformBuffer = {};
	pipeline = {};
	sampler = {};
	texture1 = {};
	texture2 = {};
}
//=============================================================================
void Demo001::OnUpdate([[maybe_unused]] float deltaTime)
{
	if (Input::IsKeyDown(GLFW_KEY_W)) camera.ProcessKeyboard(CameraForward, deltaTime);
	if (Input::IsKeyDown(GLFW_KEY_S)) camera.ProcessKeyboard(CameraBackward, deltaTime);
	if (Input::IsKeyDown(GLFW_KEY_A)) camera.ProcessKeyboard(CameraLeft, deltaTime);
	if (Input::IsKeyDown(GLFW_KEY_D)) camera.ProcessKeyboard(CameraRight, deltaTime);

	if (Input::IsMouseDown(GLFW_MOUSE_BUTTON_RIGHT))
	{
		Input::SetCursorVisible(false);
		camera.ProcessMouseMovement(Input::GetCursorOffset().x, Input::GetCursorOffset().y);
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
void Demo001::OnRender()
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

			plane.Draw();
		}

		// куб 1
		{
			uniforms.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 1.0f));
			uniformBuffer->UpdateData(uniforms);
			gl::Cmd::BindSampledImage(0, texture2.value(), sampler.value());
			gl::Cmd::BindUniformBuffer(0, uniformBuffer.value());

			box.Draw();
		}
		// куб 2
		{
			uniforms.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));
			uniformBuffer->UpdateData(uniforms);
			gl::Cmd::BindSampledImage(0, texture2.value(), sampler.value());
			gl::Cmd::BindUniformBuffer(0, uniformBuffer.value());

			box.Draw();
		}
	}
	gl::EndRendering();
}
//=============================================================================
void Demo001::OnImGuiDraw()
{
	DrawFPS();
}
//=============================================================================
void Demo001::OnResize(uint16_t width, uint16_t height)
{
	resize(width, height);
}
//=============================================================================
void Demo001::OnMouseButton([[maybe_unused]] int button, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================
void Demo001::OnMousePos([[maybe_unused]] double x, [[maybe_unused]] double y)
{
}
//=============================================================================
void Demo001::OnScroll([[maybe_unused]] double dx, [[maybe_unused]] double dy)
{
}
//=============================================================================
void Demo001::OnKey([[maybe_unused]] int key, [[maybe_unused]] int scanCode, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================