#include "stdafx.h"
#include "GameApp.h"

namespace
{
	struct UBO final
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	constexpr std::array<gl4::VertexInputBindingDescription, 3> inputBindingDescs{
	  gl4::VertexInputBindingDescription{
		.location = 0,
		.binding = 0,
		.format = gl4::Format::R32G32B32_FLOAT,
		.offset = offsetof(MeshVertex, position),
	  },
	  gl4::VertexInputBindingDescription{
		.location = 1,
		.binding = 0,
		.format = gl4::Format::R32G32B32_FLOAT,
		.offset = offsetof(MeshVertex, normal),
	  },
		gl4::VertexInputBindingDescription{
		.location = 2,
		.binding = 0,
		.format = gl4::Format::R32G32_FLOAT,
		.offset = offsetof(MeshVertex, uv),
	  },
	};

	Mesh* mesh1;
	gl4::Texture* diffuse;
	std::optional<gl4::Sampler> sampler;

	std::optional<gl4::TypedBuffer<UBO>> uniformBuffer1;
	std::optional<gl4::GraphicsPipeline> pipeline;
	std::optional<gl4::Texture> msColorTex;
	std::optional<gl4::Texture> gDepth;

	// tes mesh
	Mesh* mesh2;

	gl4::GraphicsPipeline CreatePipeline()
	{
		auto vertexShader = gl4::Shader(gl4::PipelineStage::VertexShader, FileUtils::ReadShaderCode("GameData/shaders/Minimal.vert"), "min VS");
		auto fragmentShader = gl4::Shader(gl4::PipelineStage::FragmentShader, FileUtils::ReadShaderCode("GameData/shaders/Minimal.frag"), "min FS");

		return gl4::GraphicsPipeline({
			 .name = "Triangle Pipeline",
			.vertexShader = &vertexShader,
			.fragmentShader = &fragmentShader,
			.inputAssemblyState = {.topology = gl4::PrimitiveTopology::TRIANGLE_LIST},
			.vertexInputState = {inputBindingDescs},
			.depthState = {.depthTestEnable = true},
			});
	}

	void resize(uint16_t width, uint16_t height)
	{
		// размер уменьшить на 8
		msColorTex = gl4::CreateTexture2D({ width/4u, height / 4u }, gl4::Format::R8G8B8A8_SRGB, "gAlbedo");

		gDepth = gl4::CreateTexture2D({ width, height }, gl4::Format::D32_FLOAT, "gDepth");
	}

	Camera camera;
}
//=============================================================================
EngineCreateInfo GameApp::GetCreateInfo() const
{
	return {};
}
//=============================================================================
bool GameApp::OnInit()
{
	std::vector<MeshVertex> vertices = {
		// positions            // normals            // texcoords
		{{-10.0f, -0.5f,  10.0f},  {0.0f, 1.0f, 0.0f},  { 0.0f,  0.0f}},
		{{-10.0f, -0.5f, -10.0f},  {0.0f, 1.0f, 0.0f},  { 0.0f, 10.0f}},
		{{ 10.0f, -0.5f,  10.0f},  {0.0f, 1.0f, 0.0f},  {10.0f,  0.0f}},
		{{ 10.0f, -0.5f,  10.0f},  {0.0f, 1.0f, 0.0f},  {10.0f,  0.0f}},
		{{-10.0f, -0.5f, -10.0f},  {0.0f, 1.0f, 0.0f},  { 0.0f, 10.0f}},
		{{ 10.0f, -0.5f, -10.0f},  {0.0f, 1.0f, 0.0f},  {10.0f, 10.0f}}
	};
	std::vector<uint32_t> iv = { 0, 1, 2, 3, 5 };

	mesh1 = new Mesh(vertices, iv, std::nullopt);

	{
		const aiScene* scene = aiImportFile("CoreData/mesh/Cube/Cube.gltf", ASSIMP_LOAD_FLAGS);
		if (!scene || !scene->HasMeshes())
		{
			Fatal("Not load mesh");
			return false;
		}

		const aiMesh* mesh = scene->mMeshes[0];
		std::vector<MeshVertex> mv(mesh->mNumVertices);

		for (size_t i = 0; i < mesh->mNumVertices; i++)
		{
			MeshVertex& nv = mv[i];

			nv.position.x = mesh->mVertices[i].x;
			nv.position.y = mesh->mVertices[i].y;
			nv.position.z = mesh->mVertices[i].z;

			//if (mesh->HasNormals())
			{
				nv.normal.x = mesh->mNormals[i].x;
				nv.normal.y = mesh->mNormals[i].y;
				nv.normal.z = mesh->mNormals[i].z;
			}

			//if (mesh->HasTextureCoords(0))
			{
				nv.uv.x = mesh->mTextureCoords[0][i].x;
				nv.uv.y = mesh->mTextureCoords[0][i].y;
				const auto tc = mesh->mTextureCoords[0][i];
			}
		}
		//miv.resize(mesh->mNumFaces * 3);
		std::vector<uint32_t> miv;
		for (size_t i = 0; i < mesh->mNumFaces; i++)
		{
			for (size_t j = 0; j < 3; j++)
			{
				miv.emplace_back(mesh->mFaces[i].mIndices[j]);
			}
		}

		aiReleaseImport(scene);

		mesh2 = new Mesh(mv, miv, std::nullopt);
	}

	uniformBuffer1 = gl4::TypedBuffer<UBO>(gl4::BufferStorageFlag::DynamicStorage);

	// Load Texture
	diffuse = TextureManager::GetTexture("CoreData/textures/colorful.png");

	gl4::SamplerState ss;
	ss.minFilter = gl4::MinFilter::Nearest;
	ss.magFilter = gl4::MagFilter::Nearest;
	ss.addressModeU = gl4::AddressMode::Repeat;
	ss.addressModeV = gl4::AddressMode::Repeat;
	sampler = gl4::Sampler(ss);

	pipeline = CreatePipeline();

	camera.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

	resize(GetWindowWidth(), GetWindowHeight());

	return true;
}
//=============================================================================
void GameApp::OnClose()
{
	delete mesh1;
	delete mesh2;
	uniformBuffer1 = {};
	uniformBuffer1 = {};
	pipeline = {};
	msColorTex = {};
}
//=============================================================================
void GameApp::OnUpdate(float deltaTime)
{
	if (Input::IsKeyDown(GLFW_KEY_W)) camera.ProcessKeyboard(CameraForward, deltaTime);
	if (Input::IsKeyDown(GLFW_KEY_S)) camera.ProcessKeyboard(CameraBackward, deltaTime);
	if (Input::IsKeyDown(GLFW_KEY_A)) camera.ProcessKeyboard(CameraLeft, deltaTime);
	if (Input::IsKeyDown(GLFW_KEY_D)) camera.ProcessKeyboard(CameraRight, deltaTime);

	if (Input::IsMouseDown(GLFW_MOUSE_BUTTON_RIGHT))
	{
		Input::SetCursorVisible(false);
		camera.ProcessMouseMovement(-Input::GetScreenOffset().x, Input::GetScreenOffset().y);
	}
	else if (Input::IsMouseReleased(GLFW_MOUSE_BUTTON_RIGHT))
	{
		Input::SetCursorVisible(true);
	}

	UBO ubo;
	ubo.model = glm::mat4(1.0f);
	ubo.view = camera.GetViewMatrix();
	ubo.proj = glm::perspective(glm::radians(45.0f), GetWindowAspect(), 0.01f, 1000.0f);

	uniformBuffer1->UpdateData(ubo);
}
//=============================================================================
void GameApp::OnRender()
{
	auto attachment = gl4::RenderColorAttachment{
		.texture = msColorTex.value(),
		.loadOp = gl4::AttachmentLoadOp::Clear,
		.clearValue = {.1f, .5f, .8f, 1.0f},
	};

	auto gDepthAttachment = gl4::RenderDepthStencilAttachment{
	  .texture = gDepth.value(),
	  .loadOp = gl4::AttachmentLoadOp::Clear,
	  .clearValue = {.depth = 1.0f},
	};

	gl4::BeginRendering({
		.colorAttachments = {&attachment, 1},
		.depthAttachment = gDepthAttachment
		});
	{
		gl4::Cmd::BindGraphicsPipeline(pipeline.value());
		gl4::Cmd::BindUniformBuffer(0, uniformBuffer1.value());
		gl4::Cmd::BindSampledImage(0, *diffuse, sampler.value());
		mesh1->Bind();

		gl4::Cmd::BindGraphicsPipeline(pipeline.value());
		mesh2->Bind();
		gl4::Cmd::BindUniformBuffer(0, uniformBuffer1.value());
		gl4::Cmd::BindSampledImage(0, *diffuse, sampler.value());
		mesh2->Bind();
	}
	gl4::EndRendering();

	gl4::BlitTextureToSwapchain(*msColorTex,
		{},
		{},
		msColorTex->Extent(),
		{ GetWindowWidth(), GetWindowHeight(), 1 },
		gl4::MagFilter::Nearest);
}
//=============================================================================
void GameApp::OnImGuiDraw()
{
	DrawFPS();
}
//=============================================================================
void GameApp::OnResize(uint16_t width, uint16_t height)
{
	resize(width, height);
}
//=============================================================================
void GameApp::OnMouseButton(int button, int action, int mods)
{
}
//=============================================================================
void GameApp::OnMousePos(double x, double y)
{
}
//=============================================================================
void GameApp::OnScroll(double dx, double dy)
{
}
//=============================================================================
void GameApp::OnKey(int key, int scanCode, int action, int mods)
{
}
//=============================================================================