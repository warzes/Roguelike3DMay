#include "stdafx.h"
#include "Example007.h"
// TODO: исправить проблемы и проверить правильность данных
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

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;

void main()
{
	fragNormal   = mat3(transpose(inverse(modelMatrix))) * vertexNormal;
	fragTexCoord = vertexTexCoord;
	fragPos      = vec3(modelMatrix * vec4(vertexPosition, 1.0));
	gl_Position  = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

layout(binding = 0) uniform sampler2D diffuseTexture;
layout(binding = 1) uniform sampler2D specularTexture;

layout(binding = 1, std140) uniform SceneBlock {
	vec3 viewPos;
};

struct Material {
	float shininess;
}; 
layout(binding = 2, std140) uniform MaterialBlock {
	Material material;
};

struct DirectionalLight {
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

layout(binding = 3, std140) uniform DirLightBlock {
	DirectionalLight dirLight;
};

#define NR_POINT_LIGHTS 4 
struct PointLights {
	vec4 position[NR_POINT_LIGHTS];
	vec4 ambient[NR_POINT_LIGHTS];
	vec4 diffuse[NR_POINT_LIGHTS];
	vec4 specular[NR_POINT_LIGHTS];
	vec4 attenuation[NR_POINT_LIGHTS];
};

struct PointLight {
	vec3 position;
	float constant;
	float linear;
	float quadratic;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

layout(binding = 4, std140) uniform PointLightBlock {
	PointLights pointLights;
};

// TODO: delete
PointLight getPointLight(int index)
{
	int i = index;
	return PointLight(
		pointLights.position[i].xyz,
		pointLights.attenuation[i].x,
		pointLights.attenuation[i].y,
		pointLights.attenuation[i].z,
		pointLights.ambient[i].xyz,
		pointLights.diffuse[i].xyz,
		pointLights.specular[i].xyz
	);
}

struct SpotLight {
	vec3 position;
	vec3 direction;
	vec3 attenuation;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float cut_off;
	float outer_cut_off;
};

struct spot_light_t {
	vec3 position;
	vec3 direction;
	float cut_off;
	float outer_cut_off;
	float constant;
	float linear;
	float quadratic;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

layout(binding = 5, std140) uniform SpotLightBlock {
	SpotLight spotLight;
};

// TODO: delete
spot_light_t getSpotLight()
{
	return spot_light_t(
		spotLight.position,
		spotLight.direction,
		spotLight.cut_off,
		spotLight.outer_cut_off,
		spotLight.attenuation.x,
		spotLight.attenuation.y,
		spotLight.attenuation.z,
		spotLight.ambient,
		spotLight.diffuse,
		spotLight.specular
	);
}

layout(location = 0) out vec4 outputColor;

vec3 calcDirLight(DirectionalLight light, vec3 normal, vec3 view_dir);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 frag_pos, vec3 view_dir);
vec3 calcSpotLight(spot_light_t light, vec3 normal, vec3 frag_pos, vec3 view_dir);

void main()
{
	// properties
	vec3 norm     = normalize(fragNormal);
	vec3 view_dir = normalize(viewPos - fragPos);

	// phase 1: Directional lighting
	vec3 result = calcDirLight(dirLight, norm, view_dir);

	// phase 2: Point lights
	for(int i = 0; i < NR_POINT_LIGHTS; ++i)
	{
		result += calcPointLight(getPointLight(i), norm, fragPos, view_dir);
	}
	// phase 3: Spot light
	result += calcSpotLight(getSpotLight(), norm, fragPos, view_dir);

	outputColor = vec4(result, 1.0);
}

vec3 calcDirLight(DirectionalLight light, vec3 normal, vec3 view_dir)
{
	vec3 light_dir = normalize(-light.direction);
	// diffuse shading
	float diff = max(dot(normal, light_dir), 0.0);
	// specular shading
	vec3 reflect_dir = reflect(-light_dir, normal);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
	// combine results
	vec3 ambient  = light.ambient  * vec3(texture(diffuseTexture, fragTexCoord));
	vec3 diffuse  = light.diffuse  * diff * vec3(texture(diffuseTexture, fragTexCoord));
	vec3 specular = light.specular * spec * vec3(texture(specularTexture, fragTexCoord));
	return (ambient + diffuse + specular);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 frag_pos, vec3 view_dir)
{
	vec3 light_dir = normalize(light.position - frag_pos);
	// diffuse shading
	float diff = max(dot(normal, light_dir), 0.0);
	// specular shading
	vec3 reflect_dir = reflect(-light_dir, normal);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
	// attenuation
	float distance    = length(light.position - frag_pos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + 
	light.quadratic * (distance * distance));
	// combine results
	vec3 ambient  = light.ambient  * vec3(texture(diffuseTexture, fragTexCoord));
	vec3 diffuse  = light.diffuse  * diff * vec3(texture(diffuseTexture, fragTexCoord));
	vec3 specular = light.specular * spec * vec3(texture(specularTexture, fragTexCoord));
	ambient  *= attenuation;
	diffuse  *= attenuation;
	specular *= attenuation;
	return (ambient + diffuse + specular);
}

vec3 calcSpotLight(spot_light_t light, vec3 normal, vec3 frag_pos, vec3 view_dir)
{
	vec3 light_dir = normalize(light.position - frag_pos);
	// diffuse shading
	float diff = max(dot(normal, light_dir), 0.0);
	// specular shading
	vec3 reflect_dir = reflect(-light_dir, normal);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
	// attenuation
	float distance = length(light.position - frag_pos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	// spotlight intensity
	float theta = dot(light_dir, normalize(-light.direction)); 
	float epsilon = light.cut_off - light.outer_cut_off;
	float intensity = clamp((theta - light.outer_cut_off) / epsilon, 0.0, 1.0);
	// combine results
	vec3 ambient = light.ambient * vec3(texture(diffuseTexture, fragTexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(diffuseTexture, fragTexCoord));
	vec3 specular = light.specular * spec * vec3(texture(specularTexture, fragTexCoord));
	ambient *= attenuation * intensity;
	diffuse *= attenuation * intensity;
	specular *= attenuation * intensity;
	return (ambient + diffuse + specular);
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
		float shininess;
	};
	Material materialData;

	struct alignas(16) DirectionalLight final
	{
		glm::aligned_vec3 direction;
		glm::aligned_vec3 ambient;
		glm::aligned_vec3 diffuse;
		glm::aligned_vec3 specular;
	};
	DirectionalLight dirLightData;

#define NR_POINT_LIGHTS 4 
	struct alignas(16) PointLights final
	{
		glm::vec4 position[NR_POINT_LIGHTS];
		glm::vec4 ambient[NR_POINT_LIGHTS];
		glm::vec4 diffuse[NR_POINT_LIGHTS];
		glm::vec4 specular[NR_POINT_LIGHTS];
		glm::vec4 attenuation[NR_POINT_LIGHTS];
	};
	PointLights pointsLightData;

	struct alignas(16) SpotLight final
	{
		glm::aligned_vec3 position;
		glm::aligned_vec3 direction;
		glm::aligned_vec3 attenuation;
		glm::aligned_vec3 ambient;
		glm::aligned_vec3 diffuse;
		glm::aligned_vec3 specular;
		float cut_off;
		float outer_cut_off;
	};
	SpotLight spotLightData;

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
EngineCreateInfo Example007::GetCreateInfo() const
{
	EngineCreateInfo createInfo{};
	return createInfo;
}
//=============================================================================
bool Example007::OnInit()
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
	m_matrixUBO = gl::Buffer(sizeof(MatrixBlock), gl::BufferStorageFlag::DynamicStorage);
	m_sceneUBO = gl::Buffer(sizeof(SceneBlock), gl::BufferStorageFlag::DynamicStorage);
	m_materialUBO = gl::Buffer(sizeof(Material), gl::BufferStorageFlag::DynamicStorage);
	m_dirLightUBO = gl::Buffer(sizeof(DirectionalLight), gl::BufferStorageFlag::DynamicStorage);
	m_pointLightUBO = gl::Buffer(sizeof(PointLights), gl::BufferStorageFlag::DynamicStorage);
	m_spotLightUBO = gl::Buffer(sizeof(SpotLight), gl::BufferStorageFlag::DynamicStorage);

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
	// load texture
	m_diffuseTexture = TextureManager::GetTexture("ExampleData/textures/container2.png", false);
	m_specTexture    = TextureManager::GetTexture("ExampleData/textures/container2_specular.png", false);

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
	m_camera.SetPosition(glm::vec3(0.0f, 0.0f, -3.0f));

	return true;
}
//=============================================================================
void Example007::OnClose()
{
	m_vertexBuffer = {};
	m_indexBuffer = {};
	m_matrixUBO = {};
	m_sceneUBO = {};
	m_materialUBO = {};
	m_dirLightUBO = {};
	m_pointLightUBO = {};
	m_spotLightUBO = {};
	m_pipeline = {};
	m_sampler = {};
	m_diffuseTexture = {};
	m_specTexture = {};
}
//=============================================================================
void Example007::OnUpdate([[maybe_unused]] float deltaTime)
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

		matrixData[i].viewMatrix = m_camera.GetViewMatrix();
		matrixData[i].projectionMatrix = glm::perspective(glm::radians(65.0f), GetWindowAspect(), 0.1f, 100.0f);
	}

	sceneData.viewPos = m_camera.Position;
	m_sceneUBO->UpdateData(sceneData);

	materialData.shininess = 32.0f;
	m_materialUBO->UpdateData(materialData);


	dirLightData.direction = { -0.2f, -1.0f, -0.3f };
	dirLightData.ambient   = { 0.05f, 0.05f, 0.05f };
	dirLightData.diffuse   = { 0.4f, 0.4f, 0.4f };
	dirLightData.specular  = { 0.5f, 0.5f, 0.5f };
	m_dirLightUBO->UpdateData(dirLightData);

	pointsLightData.position[0] = glm::vec4(0.7f, 0.2f, 2.0f, 0.0f);
	pointsLightData.ambient[0] = glm::vec4(0.01f, 0.01f, 0.01f, 0.0f);
	pointsLightData.diffuse[0] = glm::vec4(0.8f, 0.8f, 0.8f, 0.0f);
	pointsLightData.specular[0] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
	pointsLightData.attenuation[0] = glm::vec4(1.0f, 0.09f, 0.032f, 0.0f);
	pointsLightData.position[1] = glm::vec4(2.3f, -3.3f, -4.0f, 0.0f);
	pointsLightData.ambient[1] = glm::vec4(0.01f, 0.01f, 0.01f, 0.0f);
	pointsLightData.diffuse[1] = glm::vec4(0.8f, 0.8f, 0.8f, 0.0f);
	pointsLightData.specular[1] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
	pointsLightData.attenuation[1] = glm::vec4(1.0f, 0.09f, 0.032f, 0.0f);
	pointsLightData.position[2] = glm::vec4(-4.0f, 2.0f, -12.0f, 0.0f);
	pointsLightData.ambient[2] = glm::vec4(0.01f, 0.01f, 0.01f, 0.0f);
	pointsLightData.diffuse[2] = glm::vec4(0.8f, 0.8f, 0.8f, 0.0f);
	pointsLightData.specular[2] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
	pointsLightData.attenuation[2] = glm::vec4(1.0f, 0.09f, 0.032f, 0.0f);
	pointsLightData.position[3] = glm::vec4(0.0f, 0.0f, -3.0f, 0.0f);
	pointsLightData.ambient[3] = glm::vec4(0.01f, 0.01f, 0.01f, 0.0f);
	pointsLightData.diffuse[3] = glm::vec4(0.8f, 0.8f, 0.8f, 0.0f);
	pointsLightData.specular[3] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
	pointsLightData.attenuation[3] = glm::vec4(1.0f, 0.09f, 0.032f, 0.0f);
	m_pointLightUBO->UpdateData(pointsLightData);

	spotLightData.position = m_camera.Position;
	spotLightData.direction = m_camera.Position + m_camera.Front;
	spotLightData.cut_off = cosf(glm::radians(12.5f));
	spotLightData.outer_cut_off = cosf(glm::radians(15.0f));
	spotLightData.attenuation = glm::vec3(1.0f, 0.09f, 0.032f);
	spotLightData.ambient = glm::vec3(0.0f, 0.0f, 0.0f);
	spotLightData.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
	spotLightData.specular = glm::vec3(1.0f, 1.0f, 1.0f);
	m_spotLightUBO->UpdateData(spotLightData);
}
//=============================================================================
void Example007::OnRender()
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
		gl::Cmd::BindSampledImage(0, *m_diffuseTexture, *m_sampler);
		gl::Cmd::BindSampledImage(1, *m_specTexture, *m_sampler);

		gl::Cmd::BindUniformBuffer(1, *m_sceneUBO);
		gl::Cmd::BindUniformBuffer(2, *m_materialUBO);
		gl::Cmd::BindUniformBuffer(3, *m_dirLightUBO);
		gl::Cmd::BindUniformBuffer(4, *m_pointLightUBO);
		gl::Cmd::BindUniformBuffer(5, *m_spotLightUBO);

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
void Example007::OnImGuiDraw()
{
	DrawFPS();
}
//=============================================================================
void Example007::OnResize([[maybe_unused]] uint16_t width, [[maybe_unused]] uint16_t height)
{
}
//=============================================================================
void Example007::OnMouseButton([[maybe_unused]] int button, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================
void Example007::OnMousePos([[maybe_unused]] double x, [[maybe_unused]] double y)
{
}
//=============================================================================
void Example007::OnScroll([[maybe_unused]] double dx, [[maybe_unused]] double dy)
{
}
//=============================================================================
void Example007::OnKey([[maybe_unused]] int key, [[maybe_unused]] int scanCode, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================