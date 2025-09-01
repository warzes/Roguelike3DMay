#include "stdafx.h"
#include "Demo001.h"

//https://github.com/e-hat/efgl/blob/main/efgl/shaders/phong.glsl
//https://www.youtube.com/watch?v=Ey8fagfIm5o
//https://developer.nvidia.com/ue4-sun-temple
//https://github.com/NVIDIA-RTX/Donut
//https://sibras.github.io/OpenGL4-Tutorials/docs/Tutorials/02-Tutorial2/
//http://www.3dcpptutorials.sk/index.php?id=14

//=============================================================================
// 
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

layout(binding = 0, std140) uniform SceneBlock {
	mat4 viewMatrix;
	mat4 projectionMatrix;
};

layout(binding = 1, std140) uniform ModelMatricesBlock {
	mat4 modelMatrix;
};

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec4 fragWorldPosition;

void main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);

	// Output all out variables
	fragTexCoord      = vertexTexCoord;
	fragNormal        = mat3(transpose(inverse(modelMatrix))) * vertexNormal;
	fragWorldPosition = modelMatrix * vec4(vertexPosition, 1.0);
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

//=============================================================
// AmbientLight.glsl
//=============================================================

struct AmbientLight
{
	vec3 color;
	bool isOn;
};

vec3 getAmbientLightColor(AmbientLight ambientLight)
{
	return ambientLight.isOn ? ambientLight.color : vec3(0.0, 0.0, 0.0);
}

//=============================================================
// DiffuseLight.glsl
//=============================================================

struct DiffuseLight
{
	vec3 color;
	vec3 direction;
	float factor;
	bool isOn;
};

vec3 getDiffuseLightColor(DiffuseLight diffuseLight, vec3 normal)
{
	if(!diffuseLight.isOn) {
		return vec3(0.0, 0.0, 0.0);
	}

	float finalIntensity = max(0.0, dot(normal, -diffuseLight.direction));
	finalIntensity = clamp(finalIntensity*diffuseLight.factor, 0.0, 1.0);
	return vec3(diffuseLight.color*finalIntensity);
}

//=============================================================
// SpecularHighlight.glsl
//=============================================================

struct Material
{
	bool isEnabled;
	float specularIntensity;
	float specularPower;
};

vec3 getSpecularHighlightColor(vec3 worldPosition, vec3 normal, vec3 eyePosition, Material material, DiffuseLight diffuseLight)
{
	if(!material.isEnabled) {
		return vec3(0.0);
	}

	vec3 reflectedVector = normalize(reflect(diffuseLight.direction, normal));
	vec3 worldToEyeVector = normalize(eyePosition - worldPosition);
	float specularFactor = dot(worldToEyeVector, reflectedVector);

	if (specularFactor > 0)
	{
		specularFactor = pow(specularFactor, material.specularPower);
		return diffuseLight.color * material.specularIntensity * specularFactor;
	}

	return vec3(0.0);
}

//=============================================================
// PointLight.glsl
//=============================================================

struct PointLight
{
	vec3 position;
	vec3 color;
	
	float ambientFactor;

	float constantAttenuation;
	float linearAttenuation;
	float exponentialAttenuation;
	
	bool isOn;
};

vec3 getPointLightColor(const PointLight pointLight, const vec3 worldPosition, const vec3 normal)
{
	if(!pointLight.isOn) {
		return vec3(0.0);
	}

	/*vec3 lightDir = normalize(pointLight.position - worldPosition);
	float diffuseFactor = max(0.0, dot(normal, lightDir));

	float distance = length(pointLight.position - worldPosition);

	float attenuation = pointLight.constantAttenuation
		+ pointLight.linearAttenuation * distance
		+ pointLight.exponentialAttenuation * distance * distance;

	vec3 lightColor = pointLight.color * diffuseFactor / attenuation;

	return lightColor;*/
	
	vec3 positionToLightVector = worldPosition - pointLight.position;
	float distance = length(positionToLightVector);
	positionToLightVector = normalize(positionToLightVector);
	
	float diffuseFactor = max(0.0, dot(normal, -positionToLightVector));
	float totalAttenuation = pointLight.constantAttenuation
		+ pointLight.linearAttenuation * distance
		+ pointLight.exponentialAttenuation * pow(distance, 2.0);

	return pointLight.color * (pointLight.ambientFactor + diffuseFactor) / totalAttenuation;
}

//=============================================================
// Fragment Code
//=============================================================

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec4 fragWorldPosition;

layout(binding = 0) uniform sampler2D diffuseTexture;

layout(binding = 2, std140) uniform LightSceneBlock {
	vec4 color; // TODO: diffuse
	AmbientLight ambientLight;
	DiffuseLight diffuseLight;
	Material material;
	vec3 eyePosition;
};

layout(binding = 3, std140) uniform PointLightsBlock {
	int numPointLights;
	PointLight pointLights[100];
};

layout(location = 0) out vec4 outputColor;

void main()
{
	vec3 normal = normalize(fragNormal);
	vec4 textureColor = texture(diffuseTexture, fragTexCoord);
	vec4 objectColor = textureColor*color;
	
	vec3 ambientColor = getAmbientLightColor(ambientLight);
	vec3 diffuseColor = getDiffuseLightColor(diffuseLight, normal);
	vec3 specularHighlightColor = getSpecularHighlightColor(fragWorldPosition.xyz, normal, eyePosition, material, diffuseLight);
	vec3 lightColor = ambientColor + diffuseColor + specularHighlightColor;

	for(int i = 0; i < numPointLights; i++) {
		lightColor += getPointLightColor(pointLights[i], fragWorldPosition.xyz, normal);
	}

	outputColor = objectColor * vec4(lightColor, 1.0);

//outputColor = vec4(fragNormal * 0.5 + 0.5, 1.0);
}
)";

	struct alignas(16) SceneBlockUniform final
	{
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
	};
	SceneBlockUniform sceneBlockUniform;
	std::optional<gl::Buffer> sceneBlockUBO;

	struct alignas(16) ModelMatricesBlock final
	{
		glm::mat4 modelMatrix;
	};
	ModelMatricesBlock modelMatricesBlock;
	std::optional<gl::Buffer> modelMatricesBlockUBO;

	struct alignas(16) AmbientLight final
	{
		glm::vec3 color;
		bool isOn;
	};

	struct alignas(16) DiffuseLight final
	{
		glm::vec3 color;
		float pad0;
		glm::vec3 direction;
		float pad1;

		float factor;
		bool isOn;
	};

	struct alignas(16) Material final
	{
		bool isEnabled;
		float specularIntensity;
		float specularPower;
	};

	struct alignas(16) LightSceneBlock final
	{
		glm::vec4 color; // TODO: diffuse
		AmbientLight ambientLight;
		DiffuseLight diffuseLight;
		Material material;
		glm::vec3 eyePosition;
	};
	LightSceneBlock lightSceneBlock;
	std::optional<gl::Buffer> lightSceneBlockUBO;

	struct alignas(16) PointLight final
	{
		glm::vec3 position;
		float pad0;

		glm::vec3 color;
		float ambientFactor;

		float constantAttenuation;
		float linearAttenuation;
		float exponentialAttenuation;

		bool isOn;
	};
	struct alignas(16) PointLightsBlock final
	{
		int numPointLights;
		PointLight lights[100];
	};
	PointLightsBlock pointLightsBlock;
	std::optional<gl::Buffer> pointLightsBlockUBO;

	void setPointLightRadius(PointLight& light, float radius)
	{
		light.constantAttenuation = 1.0f;
		light.linearAttenuation = 4.5f / radius;
		light.exponentialAttenuation = 75.0f / (radius * radius);
	}

	Camera camera;

	Model plane;
	Model box;
	Model sphere;

	Model house;
	Model boxObj;

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
			.vertexInputState = { MeshVertexInputBindingDesc },
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
	plane.Create(GeometryGenerator::CreatePlane(100.0f, 100.0f, 100.0f, 100.0f));
	sphere.Create(GeometryGenerator::CreateSphere());

	auto matrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.002f));
	matrix = glm::rotate(matrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	house.Load("ExampleData/mesh/scheune_3ds/scheune.3ds", matrix);

	matrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
	matrix = glm::rotate(matrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	boxObj.Load("ExampleData/mesh/cube.obj", matrix);

	sceneBlockUBO = gl::Buffer(sizeof(SceneBlockUniform), gl::BufferStorageFlag::DynamicStorage);
	modelMatricesBlockUBO = gl::Buffer(sizeof(ModelMatricesBlock), gl::BufferStorageFlag::DynamicStorage);
	lightSceneBlockUBO = gl::Buffer(sizeof(LightSceneBlock), gl::BufferStorageFlag::DynamicStorage);
	pointLightsBlockUBO = gl::Buffer(sizeof(PointLightsBlock), gl::BufferStorageFlag::DynamicStorage);

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

	camera.SetPosition(glm::vec3(0.0f, 1.4f, -6.0f));
	camera.MovementSpeed = 20.0f;

	resize(GetWindowWidth(), GetWindowHeight());

	return true;
}
//=============================================================================
void Demo001::OnClose()
{
	box.Free();
	plane.Free();
	sphere.Free();
	house.Free();
	boxObj.Free();
	sceneBlockUBO = {};
	modelMatricesBlockUBO = {};
	lightSceneBlockUBO = {};
	pointLightsBlockUBO = {};
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

	sceneBlockUniform.viewMatrix = camera.GetViewMatrix();
	sceneBlockUniform.projectionMatrix = glm::perspective(glm::radians(65.0f), GetWindowAspect(), 0.1f, 1000.0f);
	sceneBlockUBO->UpdateData(sceneBlockUniform);

	lightSceneBlock.color = glm::vec4(1.0f);
	lightSceneBlock.ambientLight.color = glm::vec3(0.01f);
	lightSceneBlock.ambientLight.isOn = 1;

	lightSceneBlock.material.isEnabled = 1;
	lightSceneBlock.material.specularIntensity = 1.0f;
	lightSceneBlock.material.specularPower = 32.0f;

	lightSceneBlock.diffuseLight.color = glm::vec3(0.0f);
	lightSceneBlock.diffuseLight.direction = glm::vec3(0.0f);
	lightSceneBlock.diffuseLight.factor = 0.0f;
	lightSceneBlock.diffuseLight.isOn = 0;

	lightSceneBlock.eyePosition = camera.Position;

	lightSceneBlockUBO->UpdateData(lightSceneBlock);

	pointLightsBlock.numPointLights = 3;

	//for (int i = 0; i < 100; i++)
	//{

	//	pointLightsBlock.lights[i].position = glm::vec3(rand()%100-50, 1.0f, rand() % 100 - 50);
	//	pointLightsBlock.lights[i].color = glm::vec3(rand() % 255/255.0f, rand() % 255 / 255.0f, rand() % 255 / 255.0f);
	//	pointLightsBlock.lights[i].ambientFactor = 0.0f;
	//	pointLightsBlock.lights[i].isOn = 1;
	//	setPointLightRadius(pointLightsBlock.lights[i], rand() % 20+30);
	//}

	pointLightsBlock.lights[0].position = glm::vec3(0.0f, 1.0f, 0.0f);
	pointLightsBlock.lights[0].color = glm::vec3(0.0f, 1.0f, 0.0f);
	pointLightsBlock.lights[0].ambientFactor = 0.0f;
	pointLightsBlock.lights[0].constantAttenuation = 0.3f;
	pointLightsBlock.lights[0].linearAttenuation = 0.007f;
	pointLightsBlock.lights[0].exponentialAttenuation = 0.00008f;
	pointLightsBlock.lights[0].isOn = 1;
	setPointLightRadius(pointLightsBlock.lights[0], 100.0f);

	pointLightsBlock.lights[1].position = glm::vec3(0.0f, 1.0f, 20.0f);
	pointLightsBlock.lights[1].color = glm::vec3(1.0f, 0.0f, 0.0f);
	pointLightsBlock.lights[1].ambientFactor = 0.0f;
	pointLightsBlock.lights[1].constantAttenuation = 0.3f;
	pointLightsBlock.lights[1].linearAttenuation = 0.007f;
	pointLightsBlock.lights[1].exponentialAttenuation = 0.00008f;
	pointLightsBlock.lights[1].isOn = 1;
	setPointLightRadius(pointLightsBlock.lights[1], 100.0f);

	pointLightsBlock.lights[2].position = glm::vec3(0.0f, 1.0f, -20.0f);
	pointLightsBlock.lights[2].color = glm::vec3(0.2f, 0.0f, 1.0f);
	pointLightsBlock.lights[2].ambientFactor = 0.0f;
	pointLightsBlock.lights[2].constantAttenuation = 0.3f;
	pointLightsBlock.lights[2].linearAttenuation = 0.007f;
	pointLightsBlock.lights[2].exponentialAttenuation = 0.00008f;
	pointLightsBlock.lights[2].isOn = 1;
	setPointLightRadius(pointLightsBlock.lights[2], 100.0f);

	pointLightsBlockUBO->UpdateData(pointLightsBlock);
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
		gl::Cmd::BindUniformBuffer(0, sceneBlockUBO.value());
		gl::Cmd::BindUniformBuffer(2, lightSceneBlockUBO.value());
		gl::Cmd::BindUniformBuffer(3, pointLightsBlockUBO.value());

		// плоскость
		{
			modelMatricesBlock.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
			modelMatricesBlockUBO->UpdateData(modelMatricesBlock);
			gl::Cmd::BindUniformBuffer(1, modelMatricesBlockUBO.value());

			gl::Cmd::BindSampledImage(0, texture1.value(), sampler.value());
			plane.Draw(std::nullopt);
		}

		// куб из модели
		{
			modelMatricesBlock.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 1.0f, 0.0f));
			modelMatricesBlockUBO->UpdateData(modelMatricesBlock);
			gl::Cmd::BindUniformBuffer(1, modelMatricesBlockUBO.value());

			gl::Cmd::BindSampledImage(0, texture2.value(), sampler.value());
			boxObj.Draw(std::nullopt);
		}

		// куб
		{
			modelMatricesBlock.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 1.0f, 0.0f));
			modelMatricesBlockUBO->UpdateData(modelMatricesBlock);
			gl::Cmd::BindUniformBuffer(1, modelMatricesBlockUBO.value());

			gl::Cmd::BindSampledImage(0, texture2.value(), sampler.value());
			box.Draw(std::nullopt);
		}

		// Сфера
		{
			modelMatricesBlock.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 5.0f, 0.0f));

			modelMatricesBlockUBO->UpdateData(modelMatricesBlock);
			gl::Cmd::BindUniformBuffer(1, modelMatricesBlockUBO.value());

			gl::Cmd::BindSampledImage(0, texture2.value(), sampler.value());
			sphere.Draw(std::nullopt);
		}

		// Дом
		{
			std::vector<glm::vec3> housePositions
			{
				glm::vec3(0.0f, 0.0f, -10.0f),
				glm::vec3(-20.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 0.0f, 10.0f),
				glm::vec3(20.0f, 0.0f, 0.0f),
				glm::vec3(5.0f, 0.0f, 0.0f),
			};
			for (const auto& housePosition : housePositions)
			{
				modelMatricesBlock.modelMatrix = glm::translate(glm::mat4(1.0f), housePosition);

				modelMatricesBlockUBO->UpdateData(modelMatricesBlock);
				gl::Cmd::BindUniformBuffer(1, modelMatricesBlockUBO.value());

				house.Draw(sampler);
			}
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