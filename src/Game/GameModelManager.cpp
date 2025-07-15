#include "stdafx.h"
#include "GameModelManager.h"
//=============================================================================
Mesh* LoadDataMesh(const std::vector<MeshVertex>& vertex, const std::vector<uint32_t>& indices)
{
	return new Mesh(vertex, indices, std::nullopt);
}
//=============================================================================
Mesh* LoadAssimpMesh(const std::string& filename)
{
	const aiScene* scene = aiImportFile(filename.c_str(), ASSIMP_LOAD_FLAGS);
	if (!scene || !scene->HasMeshes())
	{
		Fatal("Not load mesh: " + filename);
		return nullptr;
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

		//if (mesh->HasTangentsAndBitangents())
		{
			nv.tangent.x = mesh->mTangents[i].x;
			nv.tangent.y = mesh->mTangents[i].y;
			nv.tangent.z = mesh->mTangents[i].z;
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
	return LoadDataMesh(mv, miv);
}
//=============================================================================
glm::mat4 GameModel::GetModelMat() const
{
	// TODO: сделать кеширование, если модель не двигается, то возвращать кеш

	glm::mat4 model = glm::mat4(1.0f);

	// Перемещение
	model = glm::translate(model, position);

	// Вращение вокруг осей X, Y, Z (в градусах)
	model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
	model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));

	// Масштабирование
	model = glm::scale(model, scale);
		
	return model;
}
//=============================================================================
bool GameModelManager::Init()
{
	if (!createPipeline())
		return false;

	m_globalUniformsUbo = gl4::TypedBuffer<modelUBO::GlobalUniforms>(gl4::BufferStorageFlag::DynamicStorage);
	m_objectUniformUbo = gl4::TypedBuffer<modelUBO::ObjectUniforms>(gl4::BufferStorageFlag::DynamicStorage);
	m_sceneUniformUbo = gl4::TypedBuffer<modelUBO::SceneUniforms>(gl4::BufferStorageFlag::DynamicStorage);

	gl4::SamplerState sampleDesc;
	sampleDesc.minFilter    = gl4::MinFilter::Nearest;
	sampleDesc.magFilter    = gl4::MagFilter::Nearest;
	sampleDesc.addressModeU = gl4::AddressMode::Repeat;
	sampleDesc.addressModeV = gl4::AddressMode::Repeat;
	m_nearestSampler = gl4::Sampler(sampleDesc);

	sampleDesc.anisotropy   = gl4::SampleCount::Samples16;
	sampleDesc.minFilter    = gl4::MinFilter::LinearMimapLinear;
	sampleDesc.magFilter    = gl4::MagFilter::Linear;
	sampleDesc.addressModeU = gl4::AddressMode::Repeat;
	sampleDesc.addressModeV = gl4::AddressMode::Repeat;
	m_linearSampler = gl4::Sampler(sampleDesc);

	m_models.resize(MaxModelDraw);
	m_currentModel = 0;

	m_lights.resize(4);

	m_lights[0].diffuseColor = glm::vec3(1.0f, 0.1f, 0.2f);  // Red
	m_lights[0].position = glm::vec3(4.0, 5.0, -3.0);

	m_lights[1].diffuseColor = glm::vec3(0.0f, 1.0f, 0.0f);  // Green
	m_lights[1].position = glm::vec3(3.0f, 1.0f, 3.0f);

	m_lights[2].diffuseColor = glm::vec3(0.0f, 0.0f, 1.0f);  // Blue
	m_lights[2].position = glm::vec3(-3.0f, 1.0f, -3.0f);

	m_lights[3].diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);  // White
	m_lights[3].position = glm::vec3(3.0f, 1.0f, -3.0f);

	m_lightSSBO.emplace(std::span(m_lights), gl4::BufferStorageFlag::DynamicStorage);

	return true;
}
//=============================================================================
void GameModelManager::Close()
{
	m_lightSSBO = {};
	m_globalUniformsUbo = {};
	m_objectUniformUbo = {};
	m_nearestSampler = {};
	m_linearSampler = {};
	m_pipeline = {};
}
//=============================================================================
void GameModelManager::Update(Camera& cam)
{
	modelUBO::GlobalUniforms ubo;
	ubo.view = cam.GetViewMatrix();
	ubo.proj = glm::perspective(glm::radians(65.0f), GetWindowAspect(), 0.01f, 1000.0f);
	m_globalUniformsUbo->UpdateData(ubo);
}
//=============================================================================
void GameModelManager::SetModel(GameModel* model)
{
	m_models[m_currentModel++] = model;
}
//=============================================================================
void GameModelManager::Draw()
{
	modelUBO::SceneUniforms sceneUbo;
	sceneUbo.NumLight = 1;
	m_sceneUniformUbo->UpdateData(sceneUbo);

	gl4::Cmd::BindGraphicsPipeline(m_pipeline.value());
	gl4::Cmd::BindUniformBuffer(0, m_globalUniformsUbo.value());
	gl4::Cmd::BindUniformBuffer(1, m_sceneUniformUbo.value());
	gl4::Cmd::BindStorageBuffer(0, *m_lightSSBO);

	modelUBO::ObjectUniforms trMat;
	for (size_t i = 0; i < m_currentModel; i++)
	{
		auto model = m_models[i];
		assert(model);
		trMat.model = model->GetModelMat();

		m_objectUniformUbo->UpdateData(trMat);

		gl4::Cmd::BindUniformBuffer(2, m_objectUniformUbo.value());
		gl4::Cmd::BindSampledImage(0, *model->diffuse, (model->textureFilter == gl4::MagFilter::Linear) ? m_linearSampler.value() : m_nearestSampler.value());
		model->mesh->Bind();
	}
	m_currentModel = 0;
}
//=============================================================================
bool GameModelManager::createPipeline()
{
	auto vertexShader = gl4::Shader(gl4::PipelineStage::VertexShader, FileUtils::ReadShaderCode("GameData/shaders/GameMesh.vert"), "GameMesh VS");
	if (!vertexShader.IsValid()) return false;
	auto fragmentShader = gl4::Shader(gl4::PipelineStage::FragmentShader, FileUtils::ReadShaderCode("GameData/shaders/GameMesh.frag"), "GameMesh FS");
	if (!fragmentShader.IsValid()) return false;

	m_pipeline = gl4::GraphicsPipeline({
		 .name = "Model Pipeline",
		.vertexShader = &vertexShader,
		.fragmentShader = &fragmentShader,
		.inputAssemblyState = {.topology = gl4::PrimitiveTopology::TRIANGLE_LIST},
		.vertexInputState = {MeshVertexInputBindingDescs},
		.depthState = {.depthTestEnable = true},
		});

	if (!m_pipeline.has_value()) return false;

	return true;
}
//=============================================================================