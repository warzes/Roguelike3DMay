#include "stdafx.h"
#include "Example008.h"
//=============================================================================
// TODO: заднее окно почему-то изчезает под опред углом
// TODO: возможность настраивать материал для Model 
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

layout(binding = 0, std140) uniform MatrixBlock {
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
};

layout(location = 0) out vec2 fragTexCoord;

void main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);
	fragTexCoord = vertexTexCoord;
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

layout(location = 0) in vec2 fragTexCoord;

layout(binding = 0) uniform sampler2D diffuseTexture;

layout(location = 0) out vec4 outputColor;

void main()
{
	outputColor = texture(diffuseTexture, fragTexCoord);
}
)";

	struct MatrixBlock final
	{
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
	};
	MatrixBlock matrixData;
}
//=============================================================================
EngineCreateInfo Example008::GetCreateInfo() const
{
	EngineCreateInfo createInfo{};
	return createInfo;
}
//=============================================================================
bool Example008::OnInit()
{
	//-------------------------------------------------------------------------
	// create model
	m_cube.Create(GeometryGenerator::CreateBox());
	m_plane.Create(GeometryGenerator::CreatePlane(10.0f, 10.0f, 10.0f, 10.0f));
	m_window.Create(GeometryGenerator::CreatePlane());

	//-------------------------------------------------------------------------
	// create uniform buffer
	m_uniformBuffer = gl::Buffer(sizeof(MatrixBlock), gl::BufferStorageFlag::DynamicStorage);

	//-------------------------------------------------------------------------
	// create pipeline
	auto vertexShader   = gl::Shader(gl::ShaderType::VertexShader, shaderCodeVertex, "VS");
	auto fragmentShader = gl::Shader(gl::ShaderType::FragmentShader, shaderCodeFragment, "FS");

	gl::ColorBlendState blendState{};
	blendState.attachments.push_back({});
	blendState.attachments[0].blendEnable = true;
	blendState.attachments[0].srcColorBlendFactor = gl::BlendFactor::SrcAlpha;
	blendState.attachments[0].dstColorBlendFactor = gl::BlendFactor::OneMinusSrcAlpha;
	blendState.attachments[0].colorBlendOp = gl::BlendOp::Add;
	blendState.attachments[0].srcAlphaBlendFactor = gl::BlendFactor::SrcAlpha;
	blendState.attachments[0].dstAlphaBlendFactor = gl::BlendFactor::OneMinusSrcAlpha;
	blendState.attachments[0].alphaBlendOp = gl::BlendOp::Add;

	m_pipeline = gl::GraphicsPipeline({
		.name               = "Pipeline",
		.vertexShader       = &vertexShader,
		.fragmentShader     = &fragmentShader,
		.inputAssemblyState = {.topology = gl::PrimitiveTopology::TriangleList },
		.vertexInputState   = { MeshVertexInputBindingDesc },
		.rasterizationState = {.cullMode = gl::CullMode::None },
		.depthState         = {.depthTestEnable = true },
		.colorBlendState    = blendState
		});

	//-------------------------------------------------------------------------
	// load texture
	m_texture1 = TextureManager::GetTexture("ExampleData/textures/metal.png", gl::ColorSpace::sRGB);
	m_texture2 = TextureManager::GetTexture("ExampleData/textures/marble.jpg", gl::ColorSpace::sRGB);
	m_texture3 = TextureManager::GetTexture("ExampleData/textures/transparent_window.png", gl::ColorSpace::sRGB);

	//-------------------------------------------------------------------------
	// create Sampler
	m_sampler = gl::Sampler({
		.minFilter = gl::MinFilter::Nearest,
		.magFilter = gl::MagFilter::Nearest,
		.addressModeU = gl::AddressMode::Repeat,
		.addressModeV = gl::AddressMode::Repeat,
	});

	//-------------------------------------------------------------------------
	// set camera
	m_camera.SetPosition(glm::vec3(0.0f, 0.0f, -3.0f));

	return true;
}
//=============================================================================
void Example008::OnClose()
{
	m_cube.Free();
	m_plane.Free();
	m_window.Free();
	m_uniformBuffer = {};
	m_pipeline = {};
	m_sampler = {};
	m_texture1 = nullptr;
	m_texture2 = nullptr;
	m_texture3 = nullptr;
}
//=============================================================================
void Example008::OnUpdate([[maybe_unused]] float deltaTime)
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

	matrixData.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	matrixData.viewMatrix = m_camera.GetViewMatrix();
	matrixData.projectionMatrix = glm::perspective(glm::radians(65.0f), GetWindowAspect(), 0.1f, 100.0f);
}
//=============================================================================
void Example008::OnRender()
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

		// плоскость
		{
			matrixData.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f));
			m_uniformBuffer->UpdateData(matrixData);
			gl::Cmd::BindSampledImage(0, *m_texture1, *m_sampler);
			gl::Cmd::BindUniformBuffer(0, *m_uniformBuffer);
			m_plane.Draw({});
		}

		// куб 1
		{
			matrixData.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 1.0f));
			m_uniformBuffer->UpdateData(matrixData);
			gl::Cmd::BindSampledImage(0, *m_texture2, *m_sampler);
			gl::Cmd::BindUniformBuffer(0, *m_uniformBuffer);
			m_cube.Draw({});
		}
		// куб 2
		{
			matrixData.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));
			m_uniformBuffer->UpdateData(matrixData);
			gl::Cmd::BindSampledImage(0, *m_texture2, *m_sampler);
			gl::Cmd::BindUniformBuffer(0, *m_uniformBuffer);
			m_cube.Draw({});
		}

		// окна
		{
			std::vector<glm::vec3> windowPositions
			{
				glm::vec3(-1.5f, 0.0f, 0.48f),
				glm::vec3(1.5f, 0.0f, -0.51f),
				glm::vec3(0.0f, 0.0f, -0.7f),
				glm::vec3(-0.3f, 0.0f, 2.3f),
				glm::vec3(0.5f, 0.0f, 0.6f),
			};

			glm::vec3 position = m_camera.Position;

			// simple bubble sort algorithm to sort vegetation from furthest to nearest
			for (size_t i = 1; i < windowPositions.size(); ++i)
			{
				for (int j = i - 1; j >= 0; --j)
				{
					glm::vec3 translate0 = windowPositions[j] - position;
					glm::vec3 translate1 = windowPositions[j + 1] - position;

					if (glm::length(translate0) < glm::length(translate1))
					{
						glm::vec3 temp = windowPositions[j];
						windowPositions[j] = windowPositions[j + 1];
						windowPositions[j + 1] = temp;
						break;
					}
				}
			}

			for (size_t i = 0; i < windowPositions.size(); i++)
			{
				matrixData.modelMatrix = glm::translate(glm::mat4(1.0f), windowPositions[i]);
				matrixData.modelMatrix *= glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

				m_uniformBuffer->UpdateData(matrixData);
				gl::Cmd::BindSampledImage(0, *m_texture3, *m_sampler);
				gl::Cmd::BindUniformBuffer(0, *m_uniformBuffer);
				m_window.Draw({});
			}
		}
	}
	gl::EndRendering();
}
//=============================================================================
void Example008::OnImGuiDraw()
{
	DrawFPS();
}
//=============================================================================
void Example008::OnResize([[maybe_unused]] uint16_t width, [[maybe_unused]] uint16_t height)
{
}
//=============================================================================
void Example008::OnMouseButton([[maybe_unused]] int button, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================
void Example008::OnMousePos([[maybe_unused]] double x, [[maybe_unused]] double y)
{
}
//=============================================================================
void Example008::OnScroll([[maybe_unused]] double dx, [[maybe_unused]] double dy)
{
}
//=============================================================================
void Example008::OnKey([[maybe_unused]] int key, [[maybe_unused]] int scanCode, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================