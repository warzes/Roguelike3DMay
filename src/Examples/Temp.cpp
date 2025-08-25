#include "stdafx.h"
#include "Temp.h"
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

layout(location = 0) out vec3 vFragPos;
layout(location = 1) out vec3 vNormal;
layout(location = 2) out vec2 vTexCoord;

void main()
{
	vec4 worldPos = modelMatrix * vec4(aPosition, 1.0);
	vFragPos = worldPos.xyz;
	vNormal = mat3(transpose(inverse(modelMatrix))) * aNormal;
	vTexCoord = aTexCoord;
	gl_Position = projectionMatrix * viewMatrix * worldPos;
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

layout(location = 0) in vec3 vFragPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D diffuseTex;

layout(binding = 1, std140) uniform fs_params {
	vec3 cameraPosition;
};

struct PointLight {
	vec3 position;
	vec3 color;
	float intensity;
	float constant;
	float linear;
	float quadratic;
	float padding;
};

struct SpotLight {
	vec3 position;
	vec3 direction;
	vec3 color;
	float intensity;
	float cutoff;           // cos(outer angle)
	float outerCutoff;      // cos(inner angle)
	float constant;
	float linear;
	float quadratic;
};

struct AreaLight {
	vec3 position;
	vec3 color;
	float intensity;
	float size;             // approximate size for softness
	float constant;
	float linear;
	float quadratic;
	float padding;
};

layout(binding = 2, std140) uniform fs_light {
	// Directional light
	vec3 dirLight_direction;
	vec3 dirLight_color;
	float dirLight_intensity;
	float padding1;

	// Point lights (4)
	int numPointLights;
	float padding2;
	float padding3;
	float padding4;
	PointLight pointLights[4];

	// Spot light
	SpotLight spotLight;

	// Area light (approximated as a large soft point light)
	AreaLight areaLight;
};

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
}; 
layout(binding = 3, std140) uniform fs_material {
	Material material;
};

layout(location = 0) out vec4 fragColor;

vec3 ComputeDirectionalLight(vec3 normal, vec3 viewDir);
vec3 ComputePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 ComputeSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 ComputeAreaLight(AreaLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	vec3 ViewDir = cameraPosition - vFragPos;

	vec3 norm = normalize(vNormal);
	vec3 viewDir = normalize(ViewDir);

	// Базовый цвет материала (можно смешать с текстурой)
	vec3 baseColor = material.diffuse;
	if (gl_FrontFacing) {
		baseColor = texture(diffuseTex, vTexCoord).rgb * baseColor;
	}

	vec3 result = vec3(0.0);

	// Ambient (упрощённый)
	result += baseColor * material.ambient;

	// Directional Light
	result += ComputeDirectionalLight(norm, viewDir) * baseColor;

	// Point Lights
	for (int i = 0; i < numPointLights && i < 4; i++) {
		result += ComputePointLight(pointLights[i], norm, vFragPos, viewDir) * baseColor;
	}

	// Spot Light
	result += ComputeSpotLight(spotLight, norm, vFragPos, viewDir) * baseColor;

	// Area Light (soft point light)
	//result += ComputeAreaLight(areaLight, norm, vFragPos, viewDir) * baseColor;

	fragColor = vec4(result, 1.0);
}

vec3 ComputeDirectionalLight(vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-dirLight_direction);
	float diff = max(dot(normal, lightDir), 0.0);

	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
	if (dot(normal, lightDir) <= 0.0) spec = 0.0;

	vec3 ambient = dirLight_color * dirLight_intensity * 0.1;
	vec3 diffuse = dirLight_color * dirLight_intensity * diff;
	vec3 specular = dirLight_color * dirLight_intensity * spec * 1.5;

	return ambient + diffuse + specular;
}

vec3 ComputePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = light.position - fragPos;
	float dist = length(lightDir);
	lightDir = normalize(lightDir);

	float diff = max(dot(normal, lightDir), 0.0);

	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
	if (diff <= 0.0) spec = 0.0;

	float attenuation = 1.0 / (
	light.constant +
	light.linear * dist +
	light.quadratic * (dist * dist)
	);

	vec3 ambient = light.color * light.intensity * 0.05;
	vec3 diffuse = light.color * light.intensity * diff;
	vec3 specular = light.color * light.intensity * spec * 1.2;

	return (ambient + diffuse + specular) * attenuation;
}

vec3 ComputeSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)\
{
	vec3 lightDir = light.position - fragPos;
	float dist = length(lightDir);
	lightDir = normalize(lightDir);

	// Спектральный угол
	float theta = dot(lightDir, normalize(-light.direction));
	if (theta < light.outerCutoff) return vec3(0.0);

	float epsilon = light.cutoff - light.outerCutoff;
	float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);

	float diff = max(dot(normal, lightDir), 0.0);

	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
	if (diff <= 0.0) spec = 0.0;

	float attenuation = 1.0 / (
	light.constant +
	light.linear * dist +
	light.quadratic * (dist * dist)
	);

	vec3 ambient = light.color * light.intensity * 0.1;
	vec3 diffuse = light.color * light.intensity * diff;
	vec3 specular = light.color * light.intensity * spec * 1.5;

	return (ambient + diffuse + specular) * attenuation * intensity;
}

vec3 ComputeAreaLight(AreaLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = light.position - fragPos;
	float dist = length(lightDir);
	lightDir = normalize(lightDir);

	float diff = max(dot(normal, lightDir), 0.0);

	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
	if (diff <= 0.0) spec = 0.0;

	// Более мягкое затухание для имитации "области"
	float attenuation = 1.0 / (
	light.constant +
	light.linear * dist +
	light.quadratic * (dist * dist)
	);
	attenuation *= smoothstep(0.0, 1.0, 1.0 - dist / (light.quadratic > 0.0 ? (10.0 / light.quadratic) : 100.0));

	vec3 ambient = light.color * light.intensity * 0.08;
	vec3 diffuse = light.color * light.intensity * diff * 1.2;
	vec3 specular = light.color * light.intensity * spec * 1.0;

	return (ambient + diffuse + specular) * attenuation;
}
)";

	struct vsUniforms final
	{
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
	};
	vsUniforms uniforms[10];

	struct FragUniform final
	{
		glm::vec3 cameraPosition;
	};
	FragUniform fragUniform;


	struct Lights final
	{
		// Directional light
		glm::vec3 dirLight_direction;
		glm::vec3 dirLight_color;
		float dirLight_intensity;
		float padding1;

		// Point lights (4)
		int numPointLights;
		float padding2;
		float padding3;
		float padding4;
		struct PointLight {
			glm::vec3 position;
			glm::vec3 color;
			float intensity;
			float constant;
			float linear;
			float quadratic;
			float padding;
		};
		PointLight pointLights[4];

		// Spot light
		struct SpotLight {
			glm::vec3 position;
			glm::vec3 direction;
			glm::vec3 color;
			float intensity;
			float cutoff;           // cos(outer angle)
			float outerCutoff;      // cos(inner angle)
			float constant;
			float linear;
			float quadratic;
		} spotLight;

		// Area light (approximated as a large soft point light)
		struct AreaLight {
			glm::vec3 position;
			glm::vec3 color;
			float intensity;
			float size;             // approximate size for softness
			float constant;
			float linear;
			float quadratic;
			float padding;
		} areaLight;
	};
	Lights lightUniform;

	struct Material final
	{
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;
		float shininess;
	};
	Material materialUniform;



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
	std::optional<gl::Buffer> uniformBuffer2;
	std::optional<gl::Buffer> uniformBuffer3;
	std::optional<gl::Buffer> uniformBuffer4;

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
			.inputAssemblyState = {.topology = gl::PrimitiveTopology::TriangleList },
			.vertexInputState = { inputBindingDescs },
			.depthState = {.depthTestEnable = true }
			});
	}

	void resize([[maybe_unused]] uint16_t width, [[maybe_unused]] uint16_t height)
	{
	}
}
//=============================================================================
EngineCreateInfo Temp::GetCreateInfo() const
{
	return {};
}
//=============================================================================
bool Temp::OnInit()
{
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
	uniformBuffer2 = gl::Buffer(sizeof(FragUniform), gl::BufferStorageFlag::DynamicStorage);
	uniformBuffer3 = gl::Buffer(sizeof(Lights), gl::BufferStorageFlag::DynamicStorage);
	uniformBuffer4 = gl::Buffer(sizeof(Material), gl::BufferStorageFlag::DynamicStorage);

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
void Temp::OnClose()
{
	vertexBuffer = {};
	indexBuffer = {};
	uniformBuffer = {};
	uniformBuffer2 = {};
	uniformBuffer3 = {};
	uniformBuffer4 = {};
	pipeline = {};
	sampler = {};
	texture = {};
}
//=============================================================================
void Temp::OnUpdate([[maybe_unused]] float deltaTime)
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

	uniforms[0].modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	uniforms[1].modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 5.0f, 8.0f));
	uniforms[2].modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, -2.2f, 2.5f));
	uniforms[3].modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-3.8f, -2.0f, 6.3f));
	uniforms[4].modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(2.4f, -0.4f, 3.5f));
	uniforms[5].modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-1.7f, 3.0f, 7.5f));
	uniforms[6].modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(1.3f, -2.0f, 2.5f));
	uniforms[7].modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 2.0f, 2.5f));
	uniforms[8].modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.2f, 1.5f));
	uniforms[9].modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-1.3f, 1.0f, 1.5f));
	for (size_t i = 0; i < 10; i++)
	{
		float angle = 20.0f * i;
		uniforms[i].modelMatrix *= glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

		uniforms[i].viewMatrix = camera.GetViewMatrix();
		uniforms[i].projectionMatrix = glm::perspective(glm::radians(65.0f), GetWindowAspect(), 0.1f, 100.0f);
	}

	fragUniform.cameraPosition = camera.Position;
	uniformBuffer2->UpdateData(fragUniform);


	// Направленный свет: например, "солнце" — идёт под углом в +Z (вперёд)
	lightUniform.dirLight_direction = glm::normalize(glm::vec3(1.0f, -1.0f, 1.0f));  // LHS: +Z вперёд
	lightUniform.dirLight_color = glm::vec3(1.0f, 0.9f, 0.7f);  // тёплый белый
	lightUniform.dirLight_intensity = 1.0f;

	// Точечные источники
	lightUniform.numPointLights = 4;

	// Point Light 1 — слева
	lightUniform.pointLights[0].position = glm::vec3(-2.0f, 1.0f, 0.0f);
	lightUniform.pointLights[0].color = glm::vec3(1.0f, 0.0f, 0.0f);
	lightUniform.pointLights[0].intensity = 1.5f;
	lightUniform.pointLights[0].constant = 1.0f;
	lightUniform.pointLights[0].linear = 0.09f;
	lightUniform.pointLights[0].quadratic = 0.032f;

	// Point Light 2 — справа
	lightUniform.pointLights[1].position = glm::vec3(2.0f, 1.0f, 0.0f);
	lightUniform.pointLights[1].color = glm::vec3(0.0f, 1.0f, 0.0f);
	lightUniform.pointLights[1].intensity = 1.5f;
	lightUniform.pointLights[1].constant = 1.0f;
	lightUniform.pointLights[1].linear = 0.09f;
	lightUniform.pointLights[1].quadratic = 0.032f;

	// Point Light 3 — спереди
	lightUniform.pointLights[2].position = glm::vec3(0.0f, 1.0f, 3.0f);  // +Z вперёд
	lightUniform.pointLights[2].color = glm::vec3(0.0f, 0.0f, 1.0f);
	lightUniform.pointLights[2].intensity = 1.5f;
	lightUniform.pointLights[2].constant = 1.0f;
	lightUniform.pointLights[2].linear = 0.09f;
	lightUniform.pointLights[2].quadratic = 0.032f;

	// Point Light 4 — сзади
	lightUniform.pointLights[3].position = glm::vec3(0.0f, 1.0f, -3.0f);  // -Z — назад
	lightUniform.pointLights[3].color = glm::vec3(1.0f, 1.0f, 0.0f);
	lightUniform.pointLights[3].intensity = 1.5f;
	lightUniform.pointLights[3].constant = 1.0f;
	lightUniform.pointLights[3].linear = 0.09f;
	lightUniform.pointLights[3].quadratic = 0.032f;

	// Spot Light — фонарик от камеры вперёд
	lightUniform.spotLight.position = fragUniform.cameraPosition;
	lightUniform.spotLight.direction = glm::vec3(0.0f, -0.2f, 1.0f);  // смотрит в +Z (вперёд)
	lightUniform.spotLight.color = glm::vec3(1.0f, 1.0f, 1.0f);
	lightUniform.spotLight.intensity = 2.0f;
	lightUniform.spotLight.cutoff = glm::cos(glm::radians(12.5f));
	lightUniform.spotLight.outerCutoff = glm::cos(glm::radians(17.5f));
	lightUniform.spotLight.constant = 1.0f;
	lightUniform.spotLight.linear = 0.09f;
	lightUniform.spotLight.quadratic = 0.032f;

	// Area Light — например, светящаяся панель на потолке
	lightUniform.areaLight.position = glm::vec3(0.0f, 4.0f, 2.0f);  // впереди и сверху
	lightUniform.areaLight.color = glm::vec3(0.9f, 0.9f, 1.0f);
	lightUniform.areaLight.intensity = 2.0f;
	lightUniform.areaLight.size = 2.0f;
	lightUniform.areaLight.constant = 1.0f;
	lightUniform.areaLight.linear = 0.045f;
	lightUniform.areaLight.quadratic = 0.0075f;

	uniformBuffer3->UpdateData(lightUniform);


	materialUniform.ambient = glm::vec3(0.1f);
	materialUniform.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
	materialUniform.specular = glm::vec3(0.5f, 0.5f, 0.5f);
	materialUniform.shininess = 32.0f;
	uniformBuffer4->UpdateData(materialUniform);
}
//=============================================================================
void Temp::OnRender()
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
		gl::Cmd::BindVertexBuffer(0, vertexBuffer.value(), 0, sizeof(Vertex));
		gl::Cmd::BindIndexBuffer(indexBuffer.value(), gl::IndexType::UInt);
		gl::Cmd::BindSampledImage(0, texture.value(), sampler.value());

		gl::Cmd::BindUniformBuffer(1, uniformBuffer2.value());
		gl::Cmd::BindUniformBuffer(2, uniformBuffer3.value());
		gl::Cmd::BindUniformBuffer(3, uniformBuffer4.value());
		for (size_t i = 0; i < 10; i++)
		{
			uniformBuffer->UpdateData(uniforms[i]);
			gl::Cmd::BindUniformBuffer(0, uniformBuffer.value());

			gl::Cmd::DrawIndexed(36, 1, 0, 0, 0);
		}
	}
	gl::EndRendering();
}
//=============================================================================
void Temp::OnImGuiDraw()
{
	DrawFPS();
}
//=============================================================================
void Temp::OnResize(uint16_t width, uint16_t height)
{
	resize(width, height);
}
//=============================================================================
void Temp::OnMouseButton([[maybe_unused]] int button, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================
void Temp::OnMousePos([[maybe_unused]] double x, [[maybe_unused]] double y)
{
}
//=============================================================================
void Temp::OnScroll([[maybe_unused]] double dx, [[maybe_unused]] double dy)
{
}
//=============================================================================
void Temp::OnKey([[maybe_unused]] int key, [[maybe_unused]] int scanCode, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================