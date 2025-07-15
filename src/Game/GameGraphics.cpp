#include "stdafx.h"
#include "GameGraphics.h"
#include "GameApp.h"
//=============================================================================
bool GameGraphics::Init(GameApp* gameApp)
{
	m_gameApp = gameApp;

	std::vector<MeshVertex> vertices = {
		// positions            // normals            // texcoords
		{{-50.0f, -0.5f,  50.0f},  {0.0f, 1.0f, 0.0f},  { 0.0f,  0.0f}},
		{{-50.0f, -0.5f, -50.0f},  {0.0f, 1.0f, 0.0f},  { 0.0f, 50.0f}},
		{{ 50.0f, -0.5f,  50.0f},  {0.0f, 1.0f, 0.0f},  {50.0f,  0.0f}},
		{{ 50.0f, -0.5f,  50.0f},  {0.0f, 1.0f, 0.0f},  {50.0f,  0.0f}},
		{{-50.0f, -0.5f, -50.0f},  {0.0f, 1.0f, 0.0f},  { 0.0f, 50.0f}},
		{{ 50.0f, -0.5f, -50.0f},  {0.0f, 1.0f, 0.0f},  {50.0f, 50.0f}}
	};
	std::vector<uint32_t> iv = { 0, 1, 2, 3, 4, 5 };

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

	{
		const aiScene* scene = aiImportFile("ExampleData/mesh/stall/stall.obj", ASSIMP_LOAD_FLAGS);
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

		mesh3 = new Mesh(mv, miv, std::nullopt);

		diffuseStall = TextureManager::GetTexture("ExampleData/mesh/stall/stallTexture.png", false);
	}

	ubo1 = gl4::TypedBuffer<game::ViewUBO>(gl4::BufferStorageFlag::DynamicStorage);
	ubo2 = gl4::TypedBuffer<game::TransformUBO>(gl4::BufferStorageFlag::DynamicStorage);

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

	return true;
}
//=============================================================================
void GameGraphics::Close()
{
	delete mesh1;
	delete mesh2;
	delete mesh3;
	ubo1 = {};
	ubo2 = {};
	pipeline = {};
	msColorTex = {};
	diffuseStall = {};
}
//=============================================================================
void GameGraphics::Update(float deltaTime)
{
	game::ViewUBO ubo;
	ubo.view = camera.GetViewMatrix();
	ubo.proj = glm::perspective(glm::radians(45.0f), GetWindowAspect(), 0.01f, 1000.0f);
	ubo1->UpdateData(ubo);
}
//=============================================================================
void GameGraphics::Render()
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
		game::TransformUBO trMat;
		trMat.model = glm::mat4(1.0f);
		ubo2->UpdateData(trMat);

		gl4::Cmd::BindGraphicsPipeline(pipeline.value());
		gl4::Cmd::BindUniformBuffer(0, ubo1.value());
		gl4::Cmd::BindUniformBuffer(1, ubo2.value());
		gl4::Cmd::BindSampledImage(0, *diffuse, sampler.value());
		mesh1->Bind();

		gl4::Cmd::BindGraphicsPipeline(pipeline.value());
		gl4::Cmd::BindUniformBuffer(0, ubo1.value());
		gl4::Cmd::BindUniformBuffer(1, ubo2.value());
		gl4::Cmd::BindSampledImage(0, *diffuse, sampler.value());
		mesh2->Bind();

		trMat.model = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.3f)), glm::vec3(10.0f, -2.0f, 0.0f));
		ubo2->UpdateData(trMat);

		gl4::Cmd::BindGraphicsPipeline(pipeline.value());
		gl4::Cmd::BindUniformBuffer(0, ubo1.value());
		gl4::Cmd::BindUniformBuffer(1, ubo2.value());
		gl4::Cmd::BindSampledImage(0, *diffuseStall, sampler.value());
		mesh3->Bind();
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
void GameGraphics::Resize(uint16_t width, uint16_t height)
{
	// размер уменьшить на 8
	msColorTex = gl4::CreateTexture2D({ width, height }, gl4::Format::R8G8B8A8_SRGB, "gAlbedo");

	gDepth = gl4::CreateTexture2D({ width, height }, gl4::Format::D32_FLOAT, "gDepth");
}
//=============================================================================