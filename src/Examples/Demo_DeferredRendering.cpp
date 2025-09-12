#include "stdafx.h"
#include "Demo_DeferredRendering.h"
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

layout(binding = 0, std140) uniform SceneDataBlock {
	mat4 viewMatrix;
	mat4 projectionMatrix;
	mat4 modelMatrix;
	vec3 cameraPosition;
};

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec4 fragWorldPosition;

void main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);

	fragTexCoord      = vertexTexCoord;
	fragNormal        = mat3(transpose(inverse(modelMatrix))) * vertexNormal;
	fragWorldPosition = modelMatrix * vec4(vertexPosition, 1.0);
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec4 fragWorldPosition;

layout(binding = 0) uniform sampler2D diffuseTexture;

layout(location = 0) out vec4 outputColor;

void main()
{
	vec4 textureColor = texture(diffuseTexture, fragTexCoord);
	outputColor = textureColor;
}
)";

	struct alignas(16) SceneDataBlock final
	{
		glm::mat4         viewMatrix;
		glm::mat4         projectionMatrix;
		glm::mat4         modelMatrix;
		glm::aligned_vec3 cameraPosition;
	};
	inline UniformsWrapper<SceneDataBlock> SceneDataUBO;

	struct alignas(16) Light final
	{
		glm::aligned_vec3 position;
		glm::aligned_vec3 color;
	};

	struct alignas(16) LightDataBlock final
	{
		Light light[1024];
		int numberOfLights;
	};
	inline UniformsWrapper<LightDataBlock> LightDataBlockUBO;
	int mNumberOfLights = 500;

	const glm::vec3 MinLightBoundarries(-14.0f, -1.0f, -10.0f);
	const glm::vec3 MaxLightBoundarries(12.0f, 10.0f, 10.0f);

	enum Modes
	{
		SHADED,
		NORMALS,
		COORDS,
		DIFFUSE,

	} ViewModes;

	Camera camera;

	Model sponza;

	std::optional<gl::Sampler> sampler;

	RenderTarget                        renderTarget;
	std::optional<gl::GraphicsPipeline> pipeline;


	RenderTarget                        gBufferRenderTarget;
	std::optional<gl::GraphicsPipeline> gBufferPipeline;

	RenderTarget                        lightRenderTarget;
	std::optional<gl::GraphicsPipeline> lightPipeline;

	void setupLights()
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(0, 1);

		for (int i = 0; i < mNumberOfLights; i++)
		{
			auto& light = LightDataBlockUBO->light[i];

			light.position[0] = MinLightBoundarries[0] + static_cast <float>(rand()) / (static_cast <float>(RAND_MAX / (MaxLightBoundarries[0] - MinLightBoundarries[0])));
			light.position[1] = MinLightBoundarries[1] + static_cast <float>(rand()) / (static_cast <float>(RAND_MAX / (MaxLightBoundarries[1] - MinLightBoundarries[1])));
			light.position[2] = MinLightBoundarries[2] + static_cast <float>(rand()) / (static_cast <float>(RAND_MAX / (MaxLightBoundarries[2] - MinLightBoundarries[2])));


			light.color = glm::vec3(
				static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
				static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
				static_cast <float> (rand()) / static_cast <float> (RAND_MAX)
			);
		}
		LightDataBlockUBO->numberOfLights = mNumberOfLights;
	}

	void updateLights()
	{
		for (int i = 0; i < mNumberOfLights; i++)
		{
			auto& light = LightDataBlockUBO->light[i];

			float min = MinLightBoundarries[1];
			float max = MaxLightBoundarries[1];

			light.position += glm::vec3(0, 1.0f, 0);

			if (light.position[1] > MaxLightBoundarries[1])
			{
				light.position[1] -= (MaxLightBoundarries[1] - MinLightBoundarries[1]);
			}
		}
		LightDataBlockUBO->numberOfLights = mNumberOfLights;
	}
}
//=============================================================================
EngineCreateInfo Demo_DeferredRendering::GetCreateInfo() const
{
	EngineCreateInfo createInfo{};
	return createInfo;
}
//=============================================================================
bool Demo_DeferredRendering::OnInit()
{
	auto matrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f));
	sponza.Load("ExampleData/mesh/Sponza/Sponza.gltf", matrix);

	SceneDataUBO.Init();
	LightDataBlockUBO.Init();

	gl::SamplerState sampleDesc;
	sampleDesc.minFilter = gl::MinFilter::Nearest;
	sampleDesc.magFilter = gl::MagFilter::Nearest;
	sampleDesc.addressModeU = gl::AddressMode::Repeat;
	sampleDesc.addressModeV = gl::AddressMode::Repeat;
	sampler = gl::Sampler(sampleDesc);

	camera.SetPosition(glm::vec3(0.0f, 1.4f, -6.0f));
	camera.MovementSpeed = 20.0f;

	std::vector<RTAttachment> gBufferColors =
	{
		{gl::Format::R8G8B8A8_SRGB, "gBuffer0", gl::AttachmentLoadOp::Clear},
		{gl::Format::R8G8B8_UNORM, "gBuffer1", gl::AttachmentLoadOp::Clear},
		{gl::Format::R8G8B8A8_UNORM, "gBuffer2", gl::AttachmentLoadOp::Clear}
	};
	gBufferRenderTarget.Init(GetWindowWidth(), GetWindowHeight(), gBufferColors, RTDAttachment{ gl::Format::D32_FLOAT, "MainPassDepth", gl::AttachmentLoadOp::Clear });
	// create gBufferPipeline
	{
		auto vertexShader = gl::Shader(gl::ShaderType::VertexShader, io::ReadShaderCode("ExampleData/shaders/Demo_DeferredRendering/render.vert"), "gBufferVS");
		auto fragmentShader = gl::Shader(gl::ShaderType::FragmentShader, io::ReadShaderCode("ExampleData/shaders/Demo_DeferredRendering/render.frag"), "gBufferFS");

		gBufferPipeline = gl::GraphicsPipeline({
			 .name = "GBufferPipeline",
			.vertexShader = &vertexShader,
			.fragmentShader = &fragmentShader,
			.inputAssemblyState = {.topology = gl::PrimitiveTopology::TriangleList },
			.vertexInputState = { MeshVertexInputBindingDesc },
			.depthState = {.depthTestEnable = true },
			});
	}


	lightRenderTarget.Init(GetWindowWidth(), GetWindowHeight(), RTAttachment{ gl::Format::R8G8B8A8_SRGB, "LightColor", gl::AttachmentLoadOp::Clear });
	// create pipeline
	{
		auto vertexShader = gl::Shader(gl::ShaderType::VertexShader, io::ReadShaderCode("ExampleData/shaders/Demo_DeferredRendering/light.vert"), "LightVS");
		auto fragmentShader = gl::Shader(gl::ShaderType::FragmentShader, io::ReadShaderCode("ExampleData/shaders/Demo_DeferredRendering/light.frag"), "LightFS");

		lightPipeline = gl::GraphicsPipeline({
			 .name = "Light",
			.vertexShader = &vertexShader,
			.fragmentShader = &fragmentShader,
			.inputAssemblyState = {.topology = gl::PrimitiveTopology::TriangleStrip },
			.depthState = {.depthTestEnable = false },
			});
	}

	renderTarget.Init(GetWindowWidth(), GetWindowHeight(),
		RTAttachment{ gl::Format::R8G8B8A8_SRGB, "MainPassColor", gl::AttachmentLoadOp::Clear },
		RTDAttachment{ gl::Format::D32_FLOAT, "MainPassDepth", gl::AttachmentLoadOp::Clear });
	// create pipeline
	{
		auto vertexShader = gl::Shader(gl::ShaderType::VertexShader, shaderCodeVertex, "VS");
		auto fragmentShader = gl::Shader(gl::ShaderType::FragmentShader, shaderCodeFragment, "FS");

		pipeline = gl::GraphicsPipeline({
			 .name = "Pipeline",
			.vertexShader = &vertexShader,
			.fragmentShader = &fragmentShader,
			.inputAssemblyState = {.topology = gl::PrimitiveTopology::TriangleList },
			.vertexInputState = { MeshVertexInputBindingDesc },
			.depthState = {.depthTestEnable = true },
			});
	}

	setupLights();

	return true;
}
//=============================================================================
void Demo_DeferredRendering::OnClose()
{
	sponza.Free();
	LightDataBlockUBO.Close();
	SceneDataUBO.Close();
	renderTarget.Close();
	gBufferRenderTarget.Close();
	lightRenderTarget.Close();
	pipeline = {};
	gBufferPipeline = {};
	lightPipeline = {};
	sampler = {};
}
//=============================================================================
void Demo_DeferredRendering::OnUpdate([[maybe_unused]] float deltaTime)
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

	SceneDataUBO->viewMatrix = camera.GetViewMatrix();
	SceneDataUBO->projectionMatrix = glm::perspective(glm::radians(60.0f), GetWindowAspect(), 0.1f, 1000.0f);
	SceneDataUBO->cameraPosition = camera.Position;
	SceneDataUBO->modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));

	updateLights();
}
//=============================================================================
void Demo_DeferredRendering::OnRender()
{
	gBufferRenderTarget.Begin({ .0f, .0f, .0f });
	{
		gl::Cmd::BindGraphicsPipeline(*gBufferPipeline);

		SceneDataUBO.Bind(0);
		sponza.Draw(sampler);
	}
	gBufferRenderTarget.End();

	lightRenderTarget.Begin({ .0f, .0f, .0f });
	{
		gl::Cmd::BindGraphicsPipeline(*lightPipeline);
		LightDataBlockUBO.Bind(0);
		gl::Cmd::BindSampledImage(0, *gBufferRenderTarget.GetColor(0), *sampler);
		gl::Cmd::BindSampledImage(1, *gBufferRenderTarget.GetColor(1), *sampler);
		gl::Cmd::BindSampledImage(2, *gBufferRenderTarget.GetColor(2), *sampler);
		gl::Cmd::Draw(4, 1, 0, 0);
	}
	lightRenderTarget.End();

	lightRenderTarget.BlitToSwapChain();

	//renderTarget.Begin({ .1f, .5f, .8f });
	//{
	//	gl::Cmd::BindGraphicsPipeline(*pipeline);

	//	SceneDataUBO.Bind(0);
	//	sponza.Draw(sampler);
	//}
	//renderTarget.End();

	//renderTarget.BlitToSwapChain();
}
//=============================================================================
void Demo_DeferredRendering::OnImGuiDraw()
{
	DrawFPS();
}
//=============================================================================
void Demo_DeferredRendering::OnResize(uint16_t width, uint16_t height)
{
	gBufferRenderTarget.SetSize(width, height);
	renderTarget.SetSize(width, height);
}
//=============================================================================
void Demo_DeferredRendering::OnMouseButton([[maybe_unused]] int button, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================
void Demo_DeferredRendering::OnMousePos([[maybe_unused]] double x, [[maybe_unused]] double y)
{
}
//=============================================================================
void Demo_DeferredRendering::OnScroll([[maybe_unused]] double dx, [[maybe_unused]] double dy)
{
}
//=============================================================================
void Demo_DeferredRendering::OnKey([[maybe_unused]] int key, [[maybe_unused]] int scanCode, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================