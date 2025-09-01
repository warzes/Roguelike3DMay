#include "stdafx.h"
#include "Example004.h"
//=============================================================================
namespace
{
	const char* shaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexCoord;

layout(binding = 0, std140) uniform MatrixBlock {
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
};

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragNormal;

void main()
{
	fragTexCoord = vertexTexCoord;
	fragNormal   = mat3(transpose(inverse(modelMatrix))) * vertexNormal;
	gl_Position  = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;

layout(binding = 0) uniform sampler2D diffuseTex;

layout(location = 0) out vec4 outputColor;

void main()
{
	outputColor = texture(diffuseTex, fragTexCoord) * vec4((normalize(fragNormal) * 0.5 + 0.5), 1.0);
}
)";

	struct alignas(16) vsUniforms final
	{
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
	};
	vsUniforms uniformsData[10];

	struct Vertex final
	{
		glm::vec3 pos;
		glm::vec3 normal;
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
			.offset = offsetof(Vertex, normal),
		},
		gl::VertexInputBindingDescription{
			.location = 2,
			.binding = 0,
			.format = gl::Format::R32G32_FLOAT,
			.offset = offsetof(Vertex, uv),
		}
	};
}
//=============================================================================
EngineCreateInfo Example004::GetCreateInfo() const
{
	EngineCreateInfo createInfo{};
	return createInfo;
}
//=============================================================================
bool Example004::OnInit()
{
	//-------------------------------------------------------------------------
	// create vertex buffer
	std::vector<Vertex> v = {
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
	m_vertexBuffer = gl::Buffer(v);

	//-------------------------------------------------------------------------
	// create index buffer
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
		.inputAssemblyState = {.topology = gl::PrimitiveTopology::TriangleList },
		.vertexInputState   = { inputBindingDesc },
		.depthState         = {.depthTestEnable = true }
	});

	//-------------------------------------------------------------------------
	// load texture
	m_texture = TextureManager::GetTexture("CoreData/textures/temp.png", false);

	//-------------------------------------------------------------------------
	// create Sampler
	m_sampler = gl::Sampler({
		.minFilter    = gl::MinFilter::Nearest,
		.magFilter    = gl::MagFilter::Nearest,
		.addressModeU = gl::AddressMode::Repeat,
		.addressModeV = gl::AddressMode::Repeat,
	});

	//-------------------------------------------------------------------------
	// set camera
	m_camera.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

	return true;
}
//=============================================================================
void Example004::OnClose()
{
	m_vertexBuffer = {};
	m_indexBuffer = {};
	m_uniformBuffer = {};
	m_pipeline = {};
	m_sampler = {};
	m_texture = nullptr;
}
//=============================================================================
void Example004::OnUpdate([[maybe_unused]] float deltaTime)
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

	const std::vector<glm::vec3> boxPositions
	{
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(2.0f, 5.0f, 8.0f),
		glm::vec3(-1.5f, -2.2f, 2.5f),
		glm::vec3(-3.8f, -2.0f, 6.3f),
		glm::vec3(2.4f, -0.4f, 3.5f),
		glm::vec3(-1.7f, 3.0f, 7.5f),
		glm::vec3(1.3f, -2.0f, 2.5f),
		glm::vec3(1.5f, 2.0f, 2.5f),
		glm::vec3(1.5f, 0.2f, 1.5f),
		glm::vec3(-1.3f, 1.0f, 1.5f),
	};

	for (size_t i = 0; i < boxPositions.size(); i++)
	{
		float angle = 20.0f * (float)i;

		uniformsData[i].modelMatrix = glm::translate(glm::mat4(1.0f), boxPositions[i]);
		uniformsData[i].modelMatrix *= glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

		uniformsData[i].viewMatrix       = m_camera.GetViewMatrix();
		uniformsData[i].projectionMatrix = glm::perspective(glm::radians(65.0f), GetWindowAspect(), 0.1f, 100.0f);
	}
}
//=============================================================================
void Example004::OnRender()
{
	const gl::SwapChainRenderInfo renderInfo {
		.name = "Render",
		.viewport = {.drawRect{.offset = {0, 0}, .extent = {GetWindowWidth(), GetWindowHeight()}}},
		.colorLoadOp = gl::AttachmentLoadOp::Clear,
		.clearColorValue = { 0.1f, 0.5f, 0.8f, 1.0f },
		.depthLoadOp = gl::AttachmentLoadOp::Clear,
		.clearDepthValue = 1.0f,
	};
	gl::BeginSwapChainRendering(renderInfo);
	{
		gl::Cmd::BindGraphicsPipeline(*m_pipeline);
		gl::Cmd::BindVertexBuffer(0, *m_vertexBuffer, 0, sizeof(Vertex));
		gl::Cmd::BindIndexBuffer(*m_indexBuffer, gl::IndexType::UInt);
		gl::Cmd::BindSampledImage(0, *m_texture, *m_sampler);

		for (size_t i = 0; i < 10; i++)
		{
			m_uniformBuffer->UpdateData(uniformsData[i]);
			gl::Cmd::BindUniformBuffer(0, *m_uniformBuffer);
			gl::Cmd::DrawIndexed(36, 1, 0, 0, 0);
		}
	}
	gl::EndRendering();
}
//=============================================================================
void Example004::OnImGuiDraw()
{
	DrawFPS();
}
//=============================================================================
void Example004::OnResize([[maybe_unused]] uint16_t width, [[maybe_unused]] uint16_t height)
{
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