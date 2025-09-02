#include "stdafx.h"
#include "Demo001.h"
//=============================================================================
namespace
{
#include "DemoShader.h"

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
}
//=============================================================================
EngineCreateInfo Demo001::GetCreateInfo() const
{
	return {};
}
//=============================================================================
bool Demo001::OnInit()
{
	if (!m_sceneManager.Init())
		return false;

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

	m_renderPass.SetName("FBOColor", "FBODepth");
	OnResize(GetWindowWidth(), GetWindowHeight());

	return true;
}
//=============================================================================
void Demo001::OnClose()
{
	m_renderPass.Close();
	m_sceneManager.Close();
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
	lightSceneBlock.ambientLight.color = glm::vec3(0.05f);
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
	m_renderPass.Begin({ 0.1f, 0.5f, 0.8f });
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

			gl::Cmd::BindSampledImage(0, *texture1, sampler.value());
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
	m_renderPass.End();

	m_renderPass.BlitToSwapChain();
}
//=============================================================================
void Demo001::OnImGuiDraw()
{
	DrawFPS();
}
//=============================================================================
void Demo001::OnResize(uint16_t width, uint16_t height)
{
	m_renderPass.SetSize(width, height);
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