#include "stdafx.h"
#include "GameModelManager.h"
//=============================================================================
bool GameModelManager::Init()
{
	if (!createPipeline())
		return false;

	m_globalUniformsUbo = gl4::TypedBuffer<modelUBO::GlobalUniforms>(gl4::BufferStorageFlag::DynamicStorage);
	m_objectUniformUbo = gl4::TypedBuffer<modelUBO::ObjectUniforms>(gl4::BufferStorageFlag::DynamicStorage);

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
	gl4::Cmd::BindGraphicsPipeline(m_pipeline.value());
	gl4::Cmd::BindUniformBuffer(0, m_globalUniformsUbo.value());

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