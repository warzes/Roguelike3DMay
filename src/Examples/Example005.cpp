#include "stdafx.h"
#include "Example005.h"
//=============================================================================
namespace
{
	const char* shaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;

layout(binding = 0, std140) uniform MatrixBlock {
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
};

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;

void main()
{
	fragNormal  = mat3(transpose(inverse(modelMatrix))) * vertexNormal;
	fragPos     = vec3(modelMatrix * vec4(vertexPosition, 1.0));
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
}; 

struct Light {
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

layout(binding = 1, std140) uniform SceneBlock {
	vec3 viewPos;
};

layout(binding = 2, std140) uniform MaterialBlock {
	Material material;
};

layout(binding = 3, std140) uniform LightBlock {
	Light light;
};

layout(location = 0) out vec4 outputColor;

void main()
{
	// ambient
	vec3 ambient = light.ambient * material.ambient;

	// diffuse
	vec3 norm     = normalize(fragNormal);
	vec3 lightDir = normalize(light.position - fragPos);
	float diff    = max(dot(norm, lightDir), 0.0);
	vec3 diffuse  = light.diffuse * (diff * material.diffuse);

	// specular
	vec3 viewDir    = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec      = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular   = light.specular * (spec * material.specular);

	vec3 result = ambient + diffuse + specular;
	outputColor = vec4(result, 1.0);
}
)";

	struct alignas(16) MatrixBlock final
	{
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
	};
	MatrixBlock matrixData[10];

	struct alignas(16) SceneBlock final
	{
		glm::vec3 viewPos;
	};
	SceneBlock sceneData;

	struct alignas(16) Material final
	{
		glm::aligned_vec3 ambient;
		glm::aligned_vec3 diffuse;
		glm::aligned_vec3 specular;
		float shininess;
	};
	Material materialData;

	struct alignas(16) Light final
	{
		glm::aligned_vec3 position;
		glm::aligned_vec3 ambient;
		glm::aligned_vec3 diffuse;
		glm::aligned_vec3 specular;
	};
	Light lightData;

	struct Vertex final
	{
		glm::vec3 pos;
		glm::vec3 normal;
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
			.format = gl::Format::R32G32B32_FLOAT,
			.offset = offsetof(Vertex, normal),
		}
	};
}
//=============================================================================
EngineCreateInfo Example005::GetCreateInfo() const
{
	EngineCreateInfo createInfo{};
	return createInfo;
}
//=============================================================================
bool Example005::OnInit()
{
	//-------------------------------------------------------------------------
	// create vertex buffer
	std::vector<Vertex> v = {
		// Передняя грань (Z = 0.5) — нормаль: (0, 0, 1)
		{{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}},
		{{ 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}},
		{{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}},
		{{-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}},

		// Правая грань (X = 0.5) — нормаль: (1, 0, 0)
		{{ 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}},
		{{ 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}},
		{{ 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}},
		{{ 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}},

		// Задняя грань (Z = -0.5) — нормаль: (0, 0, -1)
		{{ 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}},
		{{-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}},
		{{-0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}},
		{{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}},

		// Левая грань (X = -0.5) — нормаль: (-1, 0, 0)
		{{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}},
		{{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}},
		{{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}},
		{{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}},

		// Верхняя грань (Y = 0.5) — нормаль: (0, 1, 0)
		{{-0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}},
		{{ 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}},
		{{ 0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}},
		{{-0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}},

		// Нижняя грань (Y = -0.5) — нормаль: (0, -1, 0)
		{{-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}},
		{{ 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}},
		{{ 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}},
		{{-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}},
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
	m_matrixUBO = gl::Buffer(sizeof(MatrixBlock), gl::BufferStorageFlag::DynamicStorage);
	m_sceneUBO = gl::Buffer(sizeof(SceneBlock), gl::BufferStorageFlag::DynamicStorage);
	m_materialUBO = gl::Buffer(sizeof(Material), gl::BufferStorageFlag::DynamicStorage);
	m_lightUBO = gl::Buffer(sizeof(Light), gl::BufferStorageFlag::DynamicStorage);

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
		.depthState         = {.depthTestEnable = true },
	});

	//-------------------------------------------------------------------------
	// set camera
	m_camera.SetPosition(glm::vec3(0.0f, 0.0f, -3.0f));

	return true;
}
//=============================================================================
void Example005::OnClose()
{
	m_vertexBuffer = {};
	m_indexBuffer = {};
	m_matrixUBO = {};
	m_sceneUBO = {};
	m_materialUBO = {};
	m_lightUBO = {};
	m_pipeline = {};
}
//=============================================================================
void Example005::OnUpdate([[maybe_unused]] float deltaTime)
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

		matrixData[i].modelMatrix = glm::translate(glm::mat4(1.0f), boxPositions[i]);
		matrixData[i].modelMatrix *= glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

		matrixData[i].viewMatrix       = m_camera.GetViewMatrix();
		matrixData[i].projectionMatrix = glm::perspective(glm::radians(65.0f), GetWindowAspect(), 0.1f, 100.0f);
	}

	sceneData.viewPos = m_camera.Position;
	m_sceneUBO->UpdateData(sceneData);

	materialData.ambient   = { 1.0f, 0.5f, 0.31f };
	materialData.diffuse   = { 1.0f, 0.5f, 0.31f };
	materialData.specular  = { 0.5f, 0.5f, 0.5f };
	materialData.shininess = 32.0f;
	m_materialUBO->UpdateData(materialData);

	glm::vec3 lightColor = {
		static_cast<float>(sin(glfwGetTime() * 2.0)),
		static_cast<float>(sin(glfwGetTime() * 0.7)),
		static_cast<float>(sin(glfwGetTime() * 1.3))
	};
	lightData.position = { 1.2f, 1.0f, -2.0f };
	lightData.ambient  = lightColor * glm::vec3(0.2f);
	lightData.diffuse  = lightColor * glm::vec3(0.5f);
	lightData.specular = glm::vec3(1.0f);
	m_lightUBO->UpdateData(lightData);
}
//=============================================================================
void Example005::OnRender()
{
	const gl::SwapChainRenderInfo renderInfo
	{
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
		
		gl::Cmd::BindUniformBuffer(1, *m_sceneUBO);
		gl::Cmd::BindUniformBuffer(2, *m_materialUBO);
		gl::Cmd::BindUniformBuffer(3, *m_lightUBO);
		for (size_t i = 0; i < 10; i++)
		{
			m_matrixUBO->UpdateData(matrixData[i]);
			gl::Cmd::BindUniformBuffer(0, *m_matrixUBO);

			gl::Cmd::DrawIndexed(36, 1, 0, 0, 0);
		}
	}
	gl::EndRendering();
}
//=============================================================================
void Example005::OnImGuiDraw()
{
	DrawFPS();
}
//=============================================================================
void Example005::OnResize([[maybe_unused]] uint16_t width, [[maybe_unused]] uint16_t height)
{
}
//=============================================================================
void Example005::OnMouseButton([[maybe_unused]] int button, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================
void Example005::OnMousePos([[maybe_unused]] double x, [[maybe_unused]] double y)
{
}
//=============================================================================
void Example005::OnScroll([[maybe_unused]] double dx, [[maybe_unused]] double dy)
{
}
//=============================================================================
void Example005::OnKey([[maybe_unused]] int key, [[maybe_unused]] int scanCode, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================