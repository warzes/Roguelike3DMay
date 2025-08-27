#include "stdafx.h"
#include "Example007.h"
//=============================================================================
// Вывод кубов на сцену с несколькими источниками света разных типов
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
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(aPosition, 1.0);
	vFragPos = vec3(modelMatrix * vec4(aPosition, 1.0));

	vNormal = mat3(transpose(inverse(modelMatrix))) * aNormal;
	vTexCoord = aTexCoord;
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

layout(location = 0) in vec3 vFragPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D diffuseTexture;
layout(binding = 1) uniform sampler2D specularTexture;

layout(binding = 1, std140) uniform fs_params {
	vec3 viewPos;
};

struct Material {
	float shininess;
}; 
layout(binding = 2, std140) uniform fs_material {
	Material material;
};

struct DirectionalLight {
	vec4 direction;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

layout(binding = 3, std140) uniform fs_dir_light {
	DirectionalLight dir_light;
};

#define NR_POINT_LIGHTS 4 
struct PointLights {
	vec4 position[NR_POINT_LIGHTS];
	vec4 ambient[NR_POINT_LIGHTS];
	vec4 diffuse[NR_POINT_LIGHTS];
	vec4 specular[NR_POINT_LIGHTS];
	vec4 attenuation[NR_POINT_LIGHTS];
};

layout(binding = 4, std140) uniform fs_point_lights {
	PointLights point_lights;
};

struct SpotLight {
	vec4 position;
	vec4 direction;
	vec4 attenuation;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float cut_off;
	float outer_cut_off;
};

layout(binding = 5, std140) uniform fs_spot_light {
	SpotLight spot_light;
};

layout(location = 0) out vec4 fragColor;

// directional light type
struct dir_light_t {
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};  

struct point_light_t {
	vec3 position;
	float constant;
	float linear;
	float quadratic;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
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

dir_light_t get_directional_light();
point_light_t get_point_light(int index);
spot_light_t get_spot_light();

vec3 calc_dir_light(dir_light_t light, vec3 normal, vec3 view_dir);
vec3 calc_point_light(point_light_t light, vec3 normal, vec3 frag_pos, vec3 view_dir);
vec3 calc_spot_light(spot_light_t light, vec3 normal, vec3 frag_pos, vec3 view_dir);

void main()
{
	// properties
	vec3 norm = normalize(vNormal);
	vec3 view_dir = normalize(viewPos - vFragPos);

	// phase 1: Directional lighting
	vec3 result = calc_dir_light(get_directional_light(), norm, view_dir);
	// phase 2: Point lights
	for(int i = 0; i < NR_POINT_LIGHTS; ++i)
	{
		result += calc_point_light(get_point_light(i), norm, vFragPos, view_dir);
	}
	// phase 3: Spot light
	result += calc_spot_light(get_spot_light(), norm, vFragPos, view_dir);

	fragColor = vec4(result, 1.0);
}

dir_light_t get_directional_light()
{
	return dir_light_t(
		dir_light.direction.xyz,
		dir_light.ambient.xyz,
		dir_light.diffuse.xyz,
		dir_light.specular.xyz
	);
}

point_light_t get_point_light(int index)
{
	int i = index;
	return point_light_t(
		point_lights.position[i].xyz,
		point_lights.attenuation[i].x,
		point_lights.attenuation[i].y,
		point_lights.attenuation[i].z,
		point_lights.ambient[i].xyz,
		point_lights.diffuse[i].xyz,
		point_lights.specular[i].xyz
	);
}

spot_light_t get_spot_light()
{
	return spot_light_t(
		spot_light.position.xyz,
		spot_light.direction.xyz,
		spot_light.cut_off,
		spot_light.outer_cut_off,
		spot_light.attenuation.x,
		spot_light.attenuation.y,
		spot_light.attenuation.z,
		spot_light.ambient.xyz,
		spot_light.diffuse.xyz,
		spot_light.specular.xyz
	);
}

vec3 calc_dir_light(dir_light_t light, vec3 normal, vec3 view_dir)
{
	vec3 light_dir = normalize(-light.direction);
	// diffuse shading
	float diff = max(dot(normal, light_dir), 0.0);
	// specular shading
	vec3 reflect_dir = reflect(-light_dir, normal);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
	// combine results
	vec3 ambient  = light.ambient  * vec3(texture(diffuseTexture, vTexCoord));
	vec3 diffuse  = light.diffuse  * diff * vec3(texture(diffuseTexture, vTexCoord));
	vec3 specular = light.specular * spec * vec3(texture(specularTexture, vTexCoord));
	return (ambient + diffuse + specular);
}

vec3 calc_point_light(point_light_t light, vec3 normal, vec3 frag_pos, vec3 view_dir)
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
	vec3 ambient  = light.ambient  * vec3(texture(diffuseTexture, vTexCoord));
	vec3 diffuse  = light.diffuse  * diff * vec3(texture(diffuseTexture, vTexCoord));
	vec3 specular = light.specular * spec * vec3(texture(specularTexture, vTexCoord));
	ambient  *= attenuation;
	diffuse  *= attenuation;
	specular *= attenuation;
	return (ambient + diffuse + specular);
}

vec3 calc_spot_light(spot_light_t light, vec3 normal, vec3 frag_pos, vec3 view_dir)
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
	vec3 ambient = light.ambient * vec3(texture(diffuseTexture, vTexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(diffuseTexture, vTexCoord));
	vec3 specular = light.specular * spec * vec3(texture(specularTexture, vTexCoord));
	ambient *= attenuation * intensity;
	diffuse *= attenuation * intensity;
	specular *= attenuation * intensity;
	return (ambient + diffuse + specular);
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
		glm::vec3 viewPos;
	};
	FragUniform fragUniform;

	struct Material final
	{
		float shininess;
	};
	Material materialUniform;

	struct DirectionalLight final
	{
		glm::vec4 direction;
		glm::vec4 ambient;
		glm::vec4 diffuse;
		glm::vec4 specular;
	};
	DirectionalLight dirLightUniform;

#define NR_POINT_LIGHTS 4 
	struct PointLights final
	{
		glm::vec4 position[NR_POINT_LIGHTS];
		glm::vec4 ambient[NR_POINT_LIGHTS];
		glm::vec4 diffuse[NR_POINT_LIGHTS];
		glm::vec4 specular[NR_POINT_LIGHTS];
		glm::vec4 attenuation[NR_POINT_LIGHTS];
	};
	PointLights pointsLightUniform;

	struct SpotLight final
	{
		glm::vec4 position;
		glm::vec4 direction;
		glm::vec4 attenuation;
		glm::vec4 ambient;
		glm::vec4 diffuse;
		glm::vec4 specular;
		float cut_off;
		float outer_cut_off;
	};
	SpotLight spotLightUniform;

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
	std::optional<gl::Buffer> uniformBuffer0;
	std::optional<gl::Buffer> uniformBuffer1;
	std::optional<gl::Buffer> uniformBuffer2;
	std::optional<gl::Buffer> uniformBuffer3;
	std::optional<gl::Buffer> uniformBuffer4;
	std::optional<gl::Buffer> uniformBuffer5;

	std::optional<gl::GraphicsPipeline> pipeline;
	std::optional<gl::Texture> texture1;
	std::optional<gl::Texture> texture2;
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
			.depthState = {.depthTestEnable = true },

			});
	}

	void resize([[maybe_unused]] uint16_t width, [[maybe_unused]] uint16_t height)
	{
	}
}
//=============================================================================
EngineCreateInfo Example007::GetCreateInfo() const
{
	return {};
}
//=============================================================================
bool Example007::OnInit()
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

	uniformBuffer0 = gl::Buffer(sizeof(vsUniforms), gl::BufferStorageFlag::DynamicStorage);
	uniformBuffer1 = gl::Buffer(sizeof(FragUniform), gl::BufferStorageFlag::DynamicStorage);
	uniformBuffer2 = gl::Buffer(sizeof(Material), gl::BufferStorageFlag::DynamicStorage);
	uniformBuffer3 = gl::Buffer(sizeof(dirLightUniform), gl::BufferStorageFlag::DynamicStorage);
	uniformBuffer4 = gl::Buffer(sizeof(pointsLightUniform), gl::BufferStorageFlag::DynamicStorage);
	uniformBuffer5 = gl::Buffer(sizeof(spotLightUniform), gl::BufferStorageFlag::DynamicStorage);

	pipeline = CreatePipeline();

	{
		int imgW, imgH, nrChannels;
		auto pixels = stbi_load("ExampleData/textures/container2.png", &imgW, &imgH, &nrChannels, 4);

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
		auto pixels = stbi_load("ExampleData/textures/container2_specular.png", &imgW, &imgH, &nrChannels, 4);

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
void Example007::OnClose()
{
	vertexBuffer = {};
	indexBuffer = {};
	uniformBuffer0 = {};
	uniformBuffer1 = {};
	uniformBuffer2 = {};
	uniformBuffer3 = {};
	uniformBuffer4 = {};
	uniformBuffer5 = {};
	pipeline = {};
	sampler = {};
	texture1 = {};
	texture2 = {};
}
//=============================================================================
void Example007::OnUpdate([[maybe_unused]] float deltaTime)
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

	uniforms[0].modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	uniforms[1].modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(1.2f, 1.0f, -2.0f));
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

	fragUniform.viewPos = camera.Position;
	uniformBuffer1->UpdateData(fragUniform);

	materialUniform.shininess = 32.0f;
	uniformBuffer2->UpdateData(materialUniform);


	dirLightUniform.direction = glm::vec4(-0.2f, -1.0f, -0.3f, 1.0f);
	dirLightUniform.ambient = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f);
	dirLightUniform.diffuse = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
	dirLightUniform.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	uniformBuffer3->UpdateData(dirLightUniform);

	pointsLightUniform.position[0] = glm::vec4(0.7f, 0.2f, 2.0f, 1.0f);
	pointsLightUniform.ambient[0] = glm::vec4(0.05f, 0.05f, 0.05f, 0.0f);
	pointsLightUniform.diffuse[0] = glm::vec4(0.8f, 0.8f, 0.8f, 0.0f);
	pointsLightUniform.specular[0] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
	pointsLightUniform.attenuation[0] = glm::vec4(1.0f, 0.09f, 0.032f, 0.0f);
	pointsLightUniform.position[1] = glm::vec4(2.3f, -3.3f, -4.0f, 1.0f);
	pointsLightUniform.ambient[1] = glm::vec4(0.05f, 0.05f, 0.05f, 0.0f);
	pointsLightUniform.diffuse[1] = glm::vec4(0.8f, 0.8f, 0.8f, 0.0f);
	pointsLightUniform.specular[1] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
	pointsLightUniform.attenuation[1] = glm::vec4(1.0f, 0.09f, 0.032f, 0.0f);
	pointsLightUniform.position[2] = glm::vec4(-4.0f, 2.0f, -12.0f, 1.0f);
	pointsLightUniform.ambient[2] = glm::vec4(0.05f, 0.05f, 0.05f, 0.0f);
	pointsLightUniform.diffuse[2] = glm::vec4(0.8f, 0.8f, 0.8f, 0.0f);
	pointsLightUniform.specular[2] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
	pointsLightUniform.attenuation[2] = glm::vec4(1.0f, 0.09f, 0.032f, 0.0f);
	pointsLightUniform.position[3] = glm::vec4(0.0f, 0.0f, -3.0f, 1.0f);
	pointsLightUniform.ambient[3] = glm::vec4(0.05f, 0.05f, 0.05f, 0.0f);
	pointsLightUniform.diffuse[3] = glm::vec4(0.8f, 0.8f, 0.8f, 0.0f);
	pointsLightUniform.specular[3] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
	pointsLightUniform.attenuation[3] = glm::vec4(1.0f, 0.09f, 0.032f, 0.0f);
	uniformBuffer4->UpdateData(pointsLightUniform);

	spotLightUniform.position = glm::vec4(camera.Position, 1.0f);
	spotLightUniform.direction = glm::vec4(camera.Position + camera.Front, 1.0);
	spotLightUniform.cut_off = cosf(glm::radians(12.5f));
	spotLightUniform.outer_cut_off = cosf(glm::radians(15.0f));
	spotLightUniform.attenuation = glm::vec4(1.0f, 0.09f, 0.032f, 1.0f);
	spotLightUniform.ambient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	spotLightUniform.diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	spotLightUniform.specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	uniformBuffer5->UpdateData(spotLightUniform);
}
//=============================================================================
void Example007::OnRender()
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
		gl::Cmd::BindSampledImage(0, texture1.value(), sampler.value());
		gl::Cmd::BindSampledImage(1, texture2.value(), sampler.value());

		gl::Cmd::BindUniformBuffer(1, uniformBuffer1.value());
		gl::Cmd::BindUniformBuffer(2, uniformBuffer2.value());
		gl::Cmd::BindUniformBuffer(3, uniformBuffer3.value());
		gl::Cmd::BindUniformBuffer(4, uniformBuffer4.value());
		gl::Cmd::BindUniformBuffer(5, uniformBuffer5.value());

		for (size_t i = 0; i < 10; i++)
		{
			uniformBuffer0->UpdateData(uniforms[i]);
			gl::Cmd::BindUniformBuffer(0, uniformBuffer0.value());

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
void Example007::OnResize(uint16_t width, uint16_t height)
{
	resize(width, height);
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