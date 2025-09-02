#include "stdafx.h"
#include "Example009.h"
//=============================================================================
namespace
{
	const char* shaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec3 vertexNormal;
layout(location = 3) in vec2 vertexTexCoord;
layout(location = 4) in vec3 vertexTangent;

layout(binding = 0, std140) uniform MatrixBlock {
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
};

layout(location = 0) out vec2 fragTexCoord;

void main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);
	fragTexCoord = vertexTexCoord;
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

layout(location = 0) in vec2 fragTexCoord;

layout(binding = 0) uniform sampler2D diffuseTexture;

layout(location = 0) out vec4 outputColor;

void main()
{
	outputColor = texture(diffuseTexture, fragTexCoord);
}
)";

	struct MatrixBlock final
	{
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
	};
	MatrixBlock matrixData;

	const char* shaderQuadCodeVertex = R"(
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

	const char* shaderQuadCodeFragment = R"(
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
EngineCreateInfo Example009::GetCreateInfo() const
{
	EngineCreateInfo createInfo{};
	return createInfo;
}
//=============================================================================
bool Example009::OnInit()
{
	//-------------------------------------------------------------------------
	// create model
	m_cube.Create(GeometryGenerator::CreateBox());
	m_plane.Create(GeometryGenerator::CreatePlane(10.0f, 10.0f, 10.0f, 10.0f));

	//-------------------------------------------------------------------------
	// create uniform buffer
	m_uniformBuffer = gl::Buffer(sizeof(MatrixBlock), gl::BufferStorageFlag::DynamicStorage);

	//-------------------------------------------------------------------------
	// create pipeline
	auto vertexShader = gl::Shader(gl::ShaderType::VertexShader, shaderCodeVertex, "VS");
	auto fragmentShader = gl::Shader(gl::ShaderType::FragmentShader, shaderCodeFragment, "FS");

	gl::ColorBlendState blendState{};
	blendState.attachments.push_back({});
	blendState.attachments[0].blendEnable = true;
	blendState.attachments[0].srcColorBlendFactor = gl::BlendFactor::SrcAlpha;
	blendState.attachments[0].dstColorBlendFactor = gl::BlendFactor::OneMinusSrcAlpha;
	blendState.attachments[0].colorBlendOp = gl::BlendOp::Add;
	blendState.attachments[0].srcAlphaBlendFactor = gl::BlendFactor::SrcAlpha;
	blendState.attachments[0].dstAlphaBlendFactor = gl::BlendFactor::OneMinusSrcAlpha;
	blendState.attachments[0].alphaBlendOp = gl::BlendOp::Add;

	m_scenePipeline = gl::GraphicsPipeline({
		.name = "Pipeline",
		.vertexShader = &vertexShader,
		.fragmentShader = &fragmentShader,
		.inputAssemblyState = {.topology = gl::PrimitiveTopology::TriangleList },
		.vertexInputState = { MeshVertexInputBindingDesc },
		.rasterizationState = {.cullMode = gl::CullMode::None },
		.depthState = {.depthTestEnable = true },
		.colorBlendState = blendState
		});

	//-------------------------------------------------------------------------
	// load texture
	m_texture1 = TextureManager::GetTexture("ExampleData/textures/metal.png", false);
	m_texture2 = TextureManager::GetTexture("ExampleData/textures/marble.jpg", false);

	//-------------------------------------------------------------------------
	// create Sampler
	m_sampler = gl::Sampler({
		.minFilter = gl::MinFilter::Nearest,
		.magFilter = gl::MagFilter::Nearest,
		.addressModeU = gl::AddressMode::Repeat,
		.addressModeV = gl::AddressMode::Repeat,
		});

	//-------------------------------------------------------------------------
	// set camera
	m_camera.SetPosition(glm::vec3(0.0f, 0.0f, -3.0f));

	//-------------------------------------------------------------------------
	// create quad vertex buffer
	std::vector<Vertex> v = {
		{{ -0.95f,  0.95f, 0.0f}, {1, 0, 0}, {0.0f, 1.0f}},
		{{ -0.95f, -0.95f, 0.0f}, {0, 1, 0}, {0.0f, 0.0f}},
		{{  0.95f, -0.95f, 0.0f}, {0, 0, 1}, {1.0f, 0.0f}},
		{{  0.95f,  0.95f, 0.0f}, {1, 1, 0}, {1.0f, 1.0f}},
	};
	m_quadvb = gl::Buffer(v);

	//-------------------------------------------------------------------------
	// create quad index buffer
	std::vector<unsigned> ind = { 0, 1, 2, 2, 3, 0 };
	m_quadib = gl::Buffer(ind);

	//-------------------------------------------------------------------------
	// create pipeline
	auto vertexQuadShader = gl::Shader(gl::ShaderType::VertexShader, shaderQuadCodeVertex, "VS");
	auto fragmentQuadShader = gl::Shader(gl::ShaderType::FragmentShader, shaderQuadCodeFragment, "FS");

	m_finalPipeline = gl::GraphicsPipeline({
		.name = "SwapChain Pipeline",
		.vertexShader = &vertexQuadShader,
		.fragmentShader = &fragmentQuadShader,
		.inputAssemblyState = {.topology = gl::PrimitiveTopology::TriangleList},
		.vertexInputState = {inputBindingDesc},
		});

	OnResize(GetWindowWidth(), GetWindowHeight());

	return true;
}
//=============================================================================
void Example009::OnClose()
{
	m_cube.Free();
	m_plane.Free();
	m_uniformBuffer = {};
	m_scenePipeline = {};
	m_sampler = {};
	m_texture1 = nullptr;
	m_texture2 = nullptr;
	m_fboColorTex = {};
	m_fboColorTex = {};
	m_quadvb = {};
	m_quadib = {};
	m_finalPipeline = {};
}
//=============================================================================
void Example009::OnUpdate([[maybe_unused]] float deltaTime)
{
	if (Input::IsKeyDown(GLFW_KEY_W)) m_camera.ProcessKeyboard(CameraForward, deltaTime);
	if (Input::IsKeyDown(GLFW_KEY_S)) m_camera.ProcessKeyboard(CameraBackward, deltaTime);
	if (Input::IsKeyDown(GLFW_KEY_A)) m_camera.ProcessKeyboard(CameraLeft, deltaTime);
	if (Input::IsKeyDown(GLFW_KEY_D)) m_camera.ProcessKeyboard(CameraRight, deltaTime);

	if (Input::IsMouseDown(GLFW_MOUSE_BUTTON_RIGHT))
	{
		Input::SetCursorVisible(false);
		m_camera.ProcessMouseMovement(Input::GetCursorOffset().x, Input::GetCursorOffset().y);
	}
	else if (Input::IsMouseReleased(GLFW_MOUSE_BUTTON_RIGHT))
	{
		Input::SetCursorVisible(true);
	}

	matrixData.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	matrixData.viewMatrix = m_camera.GetViewMatrix();
	matrixData.projectionMatrix = glm::perspective(glm::radians(65.0f), GetWindowAspect(), 0.1f, 100.0f);
}
//=============================================================================
void Example009::OnRender()
{
	auto sceneColorAttachment = gl::RenderColorAttachment{
		.texture = *m_fboColorTex,
		.loadOp = gl::AttachmentLoadOp::Clear,
		.clearValue = { 0.1f, 0.5f, 0.8f, 1.0f },
	};
	auto sceneDepthAttachment = gl::RenderDepthStencilAttachment{
		.texture = *m_fboDepthTex,
		.loadOp = gl::AttachmentLoadOp::Clear,
		.clearValue = {.depth = 1.0f},
	};
	gl::BeginRendering({ .colorAttachments = {&sceneColorAttachment, 1}, .depthAttachment = sceneDepthAttachment });
	{
		gl::Cmd::BindGraphicsPipeline(*m_scenePipeline);

		// плоскость
		{
			matrixData.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f));
			m_uniformBuffer->UpdateData(matrixData);
			gl::Cmd::BindSampledImage(0, *m_texture1, *m_sampler);
			gl::Cmd::BindUniformBuffer(0, *m_uniformBuffer);
			m_plane.Draw({});
		}

		// куб
		{
			matrixData.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
			m_uniformBuffer->UpdateData(matrixData);
			gl::Cmd::BindSampledImage(0, *m_texture2, *m_sampler);
			gl::Cmd::BindUniformBuffer(0, *m_uniformBuffer);
			m_cube.Draw({});
		}
	}
	gl::EndRendering();

	const gl::SwapChainRenderInfo renderInfo
	{
		.name = "SwapChain Pass",
		.viewport = {.drawRect{.offset = {0, 0}, .extent = {GetWindowWidth(), GetWindowHeight()}}},
		.colorLoadOp = gl::AttachmentLoadOp::Clear,
		.clearColorValue = { 0.1f, 0.5f, 0.8f, 1.0f },
		.depthLoadOp = gl::AttachmentLoadOp::Clear,
		.clearDepthValue = 1.0f,
	};
	gl::BeginSwapChainRendering(renderInfo);
	{
		gl::Cmd::BindGraphicsPipeline(*m_finalPipeline);
		gl::Cmd::BindVertexBuffer(0, *m_quadvb, 0, sizeof(Vertex));
		gl::Cmd::BindIndexBuffer(*m_quadib, gl::IndexType::UInt);
		gl::Cmd::BindSampledImage(0, *m_fboColorTex, *m_sampler);
		gl::Cmd::DrawIndexed(6, 1, 0, 0, 0);
	}
	gl::EndRendering();
}
//=============================================================================
void Example009::OnImGuiDraw()
{
	DrawFPS();
}
//=============================================================================
void Example009::OnResize(uint16_t width, uint16_t height)
{
	m_fboColorTex = gl::CreateTexture2D({ width, height }, gl::Format::R8G8B8A8_SRGB, "fboColorBuffer");
	m_fboDepthTex = gl::CreateTexture2D({ width, height }, gl::Format::D32_FLOAT, "fboDepthBuffer");
}
//=============================================================================
void Example009::OnMouseButton([[maybe_unused]] int button, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================
void Example009::OnMousePos([[maybe_unused]] double x, [[maybe_unused]] double y)
{
}
//=============================================================================
void Example009::OnScroll([[maybe_unused]] double dx, [[maybe_unused]] double dy)
{
}
//=============================================================================
void Example009::OnKey([[maybe_unused]] int key, [[maybe_unused]] int scanCode, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================