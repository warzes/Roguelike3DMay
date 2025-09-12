#include "stdafx.h"
#include "Example010.h"
//=============================================================================
namespace
{
	const char* planeCodeVertex = R"(
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
layout(location = 1) out vec3 fragColor;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec4 fragWorldPosition;

void main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);

	fragColor = vec3(1.0);
	fragWorldPosition = modelMatrix * vec4(vertexPosition, 1.0);
	fragNormal = mat3(transpose(inverse(modelMatrix))) * vertexNormal;
}
)";

	const char* planeCodeFragment = R"(
#version 460 core

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec4 fragWorldPosition;

layout(location = 0) out vec4 outputColor;

void main()
{
	outputColor.rgb = fragColor;
	outputColor.a = 1.0;
}
)";

	struct alignas(16) PlaneSceneDataBlock final
	{
		glm::mat4         viewMatrix;
		glm::mat4         projectionMatrix;
		glm::mat4         modelMatrix;
		glm::aligned_vec3 cameraPosition;
	};
	inline UniformsWrapper<PlaneSceneDataBlock> PlaneSceneDataUBO;

	Camera camera;

	Model plane;
	Model sphere;

	gl::Texture* texture1;
	std::optional<gl::Sampler> sampler;

	RenderTarget                        renderTarget;
	std::optional<gl::GraphicsPipeline> planePipeline;
}
//=============================================================================
EngineCreateInfo Example010::GetCreateInfo() const
{
	EngineCreateInfo createInfo{};
	return createInfo;
}
//=============================================================================
bool Example010::OnInit()
{
	plane.Create(GeometryGenerator::CreatePlane(2.0f, 2.0f, 1.0f, 1.0f));
	sphere.Create(GeometryGenerator::CreateSphere(0.5));
	
	PlaneSceneDataUBO.Init();

	texture1 = TextureManager::GetTexture("ExampleData/textures/metal.png", gl::ColorSpace::sRGB);

	gl::SamplerState sampleDesc;
	sampleDesc.minFilter = gl::MinFilter::Nearest;
	sampleDesc.magFilter = gl::MagFilter::Nearest;
	sampleDesc.addressModeU = gl::AddressMode::Repeat;
	sampleDesc.addressModeV = gl::AddressMode::Repeat;
	sampler = gl::Sampler(sampleDesc);

	camera.SetPosition(glm::vec3(0.0f, 0.4f, -2.0f));
	camera.MovementSpeed = 10.0f;

	renderTarget.Init(GetWindowWidth(), GetWindowHeight(),
		RTAttachment{ gl::Format::R8G8B8A8_SRGB, "MainPassColor", gl::AttachmentLoadOp::Clear },
		RTDAttachment{ gl::Format::D32_FLOAT, "MainPassDepth", gl::AttachmentLoadOp::Clear });
	{
		auto vertexShader = gl::Shader(gl::ShaderType::VertexShader, planeCodeVertex, "VS");
		auto fragmentShader = gl::Shader(gl::ShaderType::FragmentShader, planeCodeFragment, "FS");

		planePipeline = gl::GraphicsPipeline({
			 .name = "Pipeline",
			.vertexShader = &vertexShader,
			.fragmentShader = &fragmentShader,
			.inputAssemblyState = {.topology = gl::PrimitiveTopology::TriangleList },
			.vertexInputState = { MeshVertexInputBindingDesc },
			.depthState = {.depthTestEnable = true },
			});
	}

	return true;
}
//=============================================================================
void Example010::OnClose()
{
	plane.Free();
	sphere.Free();
	PlaneSceneDataUBO.Close();
	renderTarget.Close();
	planePipeline = {};
	sampler = {};
	texture1 = nullptr;
}
//=============================================================================
void Example010::OnUpdate([[maybe_unused]] float deltaTime)
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

	PlaneSceneDataUBO->viewMatrix = camera.GetViewMatrix();
	PlaneSceneDataUBO->projectionMatrix = glm::perspective(glm::radians(60.0f), GetWindowAspect(), 0.1f, 1000.0f);
	PlaneSceneDataUBO->cameraPosition = camera.Position;
}
//=============================================================================
void Example010::OnRender()
{
	renderTarget.Begin({ .1f, .5f, .8f });
	{
		// плоскость
		{
			gl::Cmd::BindGraphicsPipeline(*planePipeline);
			PlaneSceneDataUBO->modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
			PlaneSceneDataUBO.Bind(0);
			gl::Cmd::BindSampledImage(0, *texture1, *sampler);
			plane.Draw(std::nullopt);
		}

		// Сфера
		{
			PlaneSceneDataUBO->modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
			PlaneSceneDataUBO.Bind(0);
			gl::Cmd::BindSampledImage(0, *texture1, *sampler);
			sphere.Draw(std::nullopt);
		}
	}
	renderTarget.End();

	renderTarget.BlitToSwapChain();
}
//=============================================================================
void Example010::OnImGuiDraw()
{
	DrawFPS();
}
//=============================================================================
void Example010::OnResize(uint16_t width, uint16_t height)
{
	renderTarget.SetSize(width, height);
}
//=============================================================================
void Example010::OnMouseButton([[maybe_unused]] int button, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================
void Example010::OnMousePos([[maybe_unused]] double x, [[maybe_unused]] double y)
{
}
//=============================================================================
void Example010::OnScroll([[maybe_unused]] double dx, [[maybe_unused]] double dy)
{
}
//=============================================================================
void Example010::OnKey([[maybe_unused]] int key, [[maybe_unused]] int scanCode, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================