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

	m_viewUbo = gl4::TypedBuffer<modelUBO::ViewUBO>(gl4::BufferStorageFlag::DynamicStorage);
	m_transformUbo = gl4::TypedBuffer<modelUBO::TransformUBO>(gl4::BufferStorageFlag::DynamicStorage);

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

	return true;
}
//=============================================================================
void GameModelManager::Close()
{
	m_viewUbo = {};
	m_transformUbo = {};
	m_nearestSampler = {};
	m_linearSampler = {};
	m_pipeline = {};
}
//=============================================================================
void GameModelManager::Update(Camera& cam)
{
	modelUBO::ViewUBO ubo;
	ubo.view = cam.GetViewMatrix();
	ubo.proj = glm::perspective(glm::radians(65.0f), GetWindowAspect(), 0.01f, 1000.0f);
	m_viewUbo->UpdateData(ubo);
}
//=============================================================================
void GameModelManager::SetModel(GameModel* model)
{
	m_models[m_currentModel++] = model;
}
//=============================================================================
void GameModelManager::Draw()
{
	modelUBO::TransformUBO trMat;

	gl4::Cmd::BindGraphicsPipeline(m_pipeline.value());
	gl4::Cmd::BindUniformBuffer(0, m_viewUbo.value());

	for (size_t i = 0; i < m_currentModel; i++)
	{
		auto model = m_models[i];
		assert(model);
		trMat.model = model->GetModelMat();
		m_transformUbo->UpdateData(trMat);

		gl4::Cmd::BindUniformBuffer(1, m_transformUbo.value());
		gl4::Cmd::BindSampledImage(0, *model->diffuse, (model->textureFilter == gl4::MagFilter::Linear) ? m_linearSampler.value() : m_nearestSampler.value());
		model->mesh->Bind();
	}
	m_currentModel = 0;
}
//=============================================================================
bool GameModelManager::createPipeline()
{
	auto vertexShader = gl4::Shader(gl4::PipelineStage::VertexShader, FileUtils::ReadShaderCode("GameData/shaders/Minimal.vert"), "min VS");
	if (!vertexShader.IsValid()) return false;
	auto fragmentShader = gl4::Shader(gl4::PipelineStage::FragmentShader, FileUtils::ReadShaderCode("GameData/shaders/Minimal.frag"), "min FS");
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