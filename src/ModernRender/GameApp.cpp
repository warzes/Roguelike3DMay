#include "stdafx.h"
#include "GameApp.h"
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

	Camera camera;

	Model plane;
	Model box;
	Model sphere;

	Model house;

	gl::Texture* texture1;
	gl::Texture* texture2;
	std::optional<gl::Sampler> sampler;

	RenderTarget                        renderTarget;
	std::optional<gl::GraphicsPipeline> pipeline;
}
//=============================================================================
EngineCreateInfo GameApp::GetCreateInfo() const
{
	EngineCreateInfo createInfo{};
	return createInfo;
}
//=============================================================================
bool GameApp::OnInit()
{
	box.Create(GeometryGenerator::CreateBox());
	plane.Create(GeometryGenerator::CreatePlane(100.0f, 100.0f, 100.0f, 100.0f));
	sphere.Create(GeometryGenerator::CreateSphere());

	auto matrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.002f));
	matrix = glm::rotate(matrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	house.Load("ExampleData/mesh/scheune_3ds/scheune.3ds", matrix);

	SceneDataUBO.Init();

	texture1 = TextureManager::GetTexture("ExampleData/textures/metal.png", gl::ColorSpace::sRGB);
	texture2 = TextureManager::GetTexture("ExampleData/textures/marble.jpg", gl::ColorSpace::sRGB);

	gl::SamplerState sampleDesc;
	sampleDesc.minFilter = gl::MinFilter::Nearest;
	sampleDesc.magFilter = gl::MagFilter::Nearest;
	sampleDesc.addressModeU = gl::AddressMode::Repeat;
	sampleDesc.addressModeV = gl::AddressMode::Repeat;
	sampler = gl::Sampler(sampleDesc);

	camera.SetPosition(glm::vec3(0.0f, 1.4f, -6.0f));
	camera.MovementSpeed = 20.0f;

	renderTarget.Init(GetWindowWidth(), GetWindowHeight(),
		RTAttachment{ gl::Format::R8G8B8A8_SRGB, "MainPassColor", gl::AttachmentLoadOp::Clear },
		RTDAttachment{ gl::Format::D32_FLOAT, "MainPassDepth", gl::AttachmentLoadOp::Clear });
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

	return true;
}
//=============================================================================
void GameApp::OnClose()
{
	box.Free();
	plane.Free();
	sphere.Free();
	house.Free();
	SceneDataUBO.Close();
	renderTarget.Close();
	pipeline = {};
	sampler = {};
	texture1 = nullptr;
	texture2 = nullptr;
}
//=============================================================================
void GameApp::OnUpdate([[maybe_unused]] float deltaTime)
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
}
//=============================================================================
void GameApp::OnRender()
{
	renderTarget.Begin({ .1f, .5f, .8f });
	{
		gl::Cmd::BindGraphicsPipeline(*pipeline);

		// плоскость
		{
			SceneDataUBO->modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
			SceneDataUBO.Bind(0);
			gl::Cmd::BindSampledImage(0, *texture1, *sampler);
			plane.Draw(std::nullopt);
		}

		// куб
		{
			SceneDataUBO->modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.5f, 0.0f));
			SceneDataUBO.Bind(0);
			gl::Cmd::BindSampledImage(0, *texture2, *sampler);
			box.Draw(std::nullopt);
		}

		// Сфера
		{
			SceneDataUBO->modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 5.0f, 0.0f));
			SceneDataUBO.Bind(0);
			gl::Cmd::BindSampledImage(0, *texture2, *sampler);
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
				SceneDataUBO->modelMatrix = glm::translate(glm::mat4(1.0f), housePosition);
				SceneDataUBO.Bind(0);
				house.Draw(sampler);
			}
		}
	}
	renderTarget.End();

	renderTarget.BlitToSwapChain();
}
//=============================================================================
void GameApp::OnImGuiDraw()
{
	DrawFPS();
}
//=============================================================================
void GameApp::OnResize(uint16_t width, uint16_t height)
{
	renderTarget.SetSize(width, height);
}
//=============================================================================
void GameApp::OnMouseButton([[maybe_unused]] int button, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================
void GameApp::OnMousePos([[maybe_unused]] double x, [[maybe_unused]] double y)
{
}
//=============================================================================
void GameApp::OnScroll([[maybe_unused]] double dx, [[maybe_unused]] double dy)
{
}
//=============================================================================
void GameApp::OnKey([[maybe_unused]] int key, [[maybe_unused]] int scanCode, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================