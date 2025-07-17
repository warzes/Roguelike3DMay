#include "stdafx.h"
#include "GameModelManager.h"
#include "ShadowPassManager.h"
//=============================================================================
bool GameModelManager::Init()
{
	if (!createPipeline())
		return false;

	m_globalUniformsUbo = gl4::TypedBuffer<modelUBO::GlobalUniforms>(gl4::BufferStorageFlag::DynamicStorage);
	m_objectUniformUbo = gl4::TypedBuffer<modelUBO::ObjectUniforms>(gl4::BufferStorageFlag::DynamicStorage);
	m_materialUniformUbo = gl4::TypedBuffer<modelUBO::MaterialUniform>(gl4::BufferStorageFlag::DynamicStorage);

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
	m_currentDrawShadowModel = 0;

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
void GameModelManager::Update()
{
}
//=============================================================================
void GameModelManager::SetModel(GameModel* model)
{
	m_models[m_currentModel++] = model;
	m_currentDrawShadowModel++;
}
//=============================================================================
void GameModelManager::Draw(Camera& cam)
{
	modelUBO::GlobalUniforms ubo;
	ubo.view = cam.GetViewMatrix();
	ubo.proj = glm::perspective(glm::radians(65.0f), GetWindowAspect(), 0.01f, 1000.0f);
	m_globalUniformsUbo->UpdateData(ubo);

	gl4::Cmd::BindGraphicsPipeline(m_pipeline.value());
	gl4::Cmd::BindUniformBuffer(0, m_globalUniformsUbo.value());

	modelUBO::ObjectUniforms trMat;
	modelUBO::MaterialUniform materialUbo;
	for (size_t i = 0; i < m_currentModel; i++)
	{
		auto model = m_models[i];
		assert(model);
		
		// 
		{
			trMat.model = model->GetModelMat();
			m_objectUniformUbo->UpdateData(trMat);
		}

		// material set
		{
			auto& mat = model->material;

			materialUbo.diffuseMaterial = mat.diffuse;
			materialUbo.specularMaterial = mat.specular;
			materialUbo.shininessMaterial = mat.shininess;

			materialUbo.hasDiffuse =  mat.diffuseTexture != nullptr;
			materialUbo.hasSpecular = mat.specularTexture != nullptr;
			materialUbo.hasEmission = mat.emissionTexture != nullptr;
			materialUbo.hasNormalMap = mat.normalTexture != nullptr && false;
			materialUbo.hasDepthMap = mat.depthTexture != nullptr;
			materialUbo.emissionStrength = mat.emissionStrength;
			materialUbo.blinn = true;
			materialUbo.heightScale = mat.heightScale;

			m_materialUniformUbo->UpdateData(materialUbo);
		}

		gl4::Cmd::BindUniformBuffer(2, m_objectUniformUbo.value());
		gl4::Cmd::BindUniformBuffer(3, m_materialUniformUbo.value());

		const gl4::Sampler& sampler = (model->textureFilter == gl4::MagFilter::Linear) ? m_linearSampler.value() : m_nearestSampler.value();

		if (materialUbo.hasDiffuse)
			gl4::Cmd::BindSampledImage(0, *model->material.diffuseTexture, sampler);
		if (materialUbo.hasSpecular)
			gl4::Cmd::BindSampledImage(1, *model->material.specularTexture, sampler);
		if (materialUbo.hasEmission)
			gl4::Cmd::BindSampledImage(2, *model->material.emissionTexture, sampler);
		if (materialUbo.hasNormalMap)
			gl4::Cmd::BindSampledImage(3, *model->material.normalTexture, sampler);
		if (materialUbo.hasDepthMap)
			gl4::Cmd::BindSampledImage(4, *model->material.depthTexture, sampler);

		model->mesh->Bind();
	}
	m_currentModel = 0;
}
//=============================================================================
void GameModelManager::DrawInDepth(Camera& cam, ShadowPassManager& shadowPassMgr)
{
	modelUBO::GlobalUniforms ubo;
	ubo.view = glm::mat4(1.0f);
	ubo.proj = shadowPassMgr.GetShadowPass().lightSpaceMatrix;
	m_globalUniformsUbo->UpdateData(ubo);

	gl4::Cmd::BindGraphicsPipeline(m_pipelineInDepth.value());
	gl4::Cmd::BindUniformBuffer(0, m_globalUniformsUbo.value());

	modelUBO::ObjectUniforms trMat;	
	for (size_t i = 0; i < m_currentDrawShadowModel; i++)
	{
		auto model = m_models[i];
		assert(model);
		// 
		{
			trMat.model = model->GetModelMat();
			m_objectUniformUbo->UpdateData(trMat);
			gl4::Cmd::BindUniformBuffer(1, m_objectUniformUbo.value());
		}
		model->mesh->Bind();
	}
	m_currentDrawShadowModel = 0;
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
		.depthState = {.depthTestEnable = true, .depthWriteEnable = true},
		});

	if (!m_pipeline.has_value()) return false;
	
	auto vertexShader2 = gl4::Shader(gl4::PipelineStage::VertexShader, FileUtils::ReadShaderCode("GameData/shaders/Depth.vert"), "Depth VS");
	if (!vertexShader2.IsValid()) return false;
	auto fragmentShader2 = gl4::Shader(gl4::PipelineStage::FragmentShader, FileUtils::ReadShaderCode("GameData/shaders/Depth.frag"), "Depth FS");
	if (!fragmentShader2.IsValid()) return false;

	m_pipelineInDepth = gl4::GraphicsPipeline({
		 .name = "Model In Depth Pipeline",
		.vertexShader = &vertexShader2,
		.fragmentShader = &fragmentShader2,
		.inputAssemblyState = {.topology = gl4::PrimitiveTopology::TRIANGLE_LIST},
		.vertexInputState = {MeshVertexInputBindingDescs},
		.depthState = {.depthTestEnable = true, .depthWriteEnable = true },
		});

	if (!m_pipelineInDepth.has_value()) return false;

	return true;
}
//=============================================================================