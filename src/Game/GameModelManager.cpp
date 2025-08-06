#include "stdafx.h"
#include "GameModelManager.h"
#include "ShadowPassManager.h"
//=============================================================================
bool GameModelManager::Init()
{
	if (!createPipeline())
		return false;

	m_globalUniformsUbo = gl::TypedBuffer<modelUBO::GlobalUniforms>(gl::BufferStorageFlag::DynamicStorage);
	m_objectUniformUbo = gl::TypedBuffer<modelUBO::ObjectUniforms>(gl::BufferStorageFlag::DynamicStorage);
	m_materialUniformUbo = gl::TypedBuffer<modelUBO::MaterialUniform>(gl::BufferStorageFlag::DynamicStorage);

	gl::SamplerState sampleDesc;
	sampleDesc.minFilter    = gl::MinFilter::Nearest;
	sampleDesc.magFilter    = gl::MagFilter::Nearest;
	sampleDesc.addressModeU = gl::AddressMode::Repeat;
	sampleDesc.addressModeV = gl::AddressMode::Repeat;
	m_nearestSampler = gl::Sampler(sampleDesc);

	sampleDesc.anisotropy   = gl::SampleCount::Samples16;
	sampleDesc.minFilter    = gl::MinFilter::LinearMimapLinear;
	sampleDesc.magFilter    = gl::MagFilter::Linear;
	sampleDesc.addressModeU = gl::AddressMode::Repeat;
	sampleDesc.addressModeV = gl::AddressMode::Repeat;
	m_linearSampler = gl::Sampler(sampleDesc);

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
void GameModelManager::SetModel(GameModelOld* model)
{
	m_models[m_currentModel++] = model;
	m_currentDrawShadowModel++;
}
//=============================================================================
void GameModelManager::Draw(Camera& cam, ShadowPassManager& shadowPassMgr)
{
	modelUBO::GlobalUniforms ubo;
	ubo.view = cam.GetViewMatrix();
	ubo.proj = glm::perspective(glm::radians(65.0f), GetWindowAspect(), 0.01f, 1000.0f);
	m_globalUniformsUbo->UpdateData(ubo);

	gl::Cmd::BindGraphicsPipeline(m_pipeline.value());
	gl::Cmd::BindUniformBuffer(0, m_globalUniformsUbo.value());

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

		gl::Cmd::BindUniformBuffer(2, m_objectUniformUbo.value());
		gl::Cmd::BindUniformBuffer(3, m_materialUniformUbo.value());

		const gl::Sampler& sampler = (model->textureFilter == gl::MagFilter::Linear) ? m_linearSampler.value() : m_nearestSampler.value();

		if (materialUbo.hasDiffuse)
			gl::Cmd::BindSampledImage(0, *model->material.diffuseTexture, sampler);
		if (materialUbo.hasSpecular)
			gl::Cmd::BindSampledImage(1, *model->material.specularTexture, sampler);
		if (materialUbo.hasEmission)
			gl::Cmd::BindSampledImage(2, *model->material.emissionTexture, sampler);
		if (materialUbo.hasNormalMap)
			gl::Cmd::BindSampledImage(3, *model->material.normalTexture, sampler);
		if (materialUbo.hasDepthMap)
			gl::Cmd::BindSampledImage(4, *model->material.depthTexture, sampler);

		gl::Cmd::BindSampledImage(5, *shadowPassMgr.GetShadowPass().depthTexture, sampler);

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

	gl::Cmd::BindGraphicsPipeline(m_pipelineInDepth.value());
	gl::Cmd::BindUniformBuffer(0, m_globalUniformsUbo.value());

	modelUBO::ObjectUniforms trMat;	
	for (size_t i = 0; i < m_currentDrawShadowModel; i++)
	{
		auto model = m_models[i];
		assert(model);
		// 
		{
			trMat.model = model->GetModelMat();
			m_objectUniformUbo->UpdateData(trMat);
			gl::Cmd::BindUniformBuffer(1, m_objectUniformUbo.value());
		}
		model->mesh->Bind();
	}
	m_currentDrawShadowModel = 0;
}
//=============================================================================
bool GameModelManager::createPipeline()
{
	auto vertexShader = gl::Shader(gl::PipelineStage::VertexShader, io::ReadShaderCode("GameData/shaders/GameMesh.vert"), "GameMesh VS");
	if (!vertexShader.IsValid()) return false;
	auto fragmentShader = gl::Shader(gl::PipelineStage::FragmentShader, io::ReadShaderCode("GameData/shaders/GameMesh.frag"), "GameMesh FS");
	if (!fragmentShader.IsValid()) return false;

	m_pipeline = gl::GraphicsPipeline({
		 .name = "Model Pipeline",
		.vertexShader = &vertexShader,
		.fragmentShader = &fragmentShader,
		.inputAssemblyState = {.topology = gl::PrimitiveTopology::TRIANGLE_LIST},
		.vertexInputState = {MeshVertexInputBindingDescs},
		.depthState = {.depthTestEnable = true, .depthWriteEnable = true},
		});

	if (!m_pipeline.has_value()) return false;
	
	auto vertexShader2 = gl::Shader(gl::PipelineStage::VertexShader, io::ReadShaderCode("GameData/shaders/Depth.vert"), "Depth VS");
	if (!vertexShader2.IsValid()) return false;
	auto fragmentShader2 = gl::Shader(gl::PipelineStage::FragmentShader, io::ReadShaderCode("GameData/shaders/Depth.frag"), "Depth FS");
	if (!fragmentShader2.IsValid()) return false;

	m_pipelineInDepth = gl::GraphicsPipeline({
		 .name = "Model In Depth Pipeline",
		.vertexShader = &vertexShader2,
		.fragmentShader = &fragmentShader2,
		.inputAssemblyState = {.topology = gl::PrimitiveTopology::TRIANGLE_LIST},
		.vertexInputState = {MeshVertexInputBindingDescs},
		.depthState = {.depthTestEnable = true, .depthWriteEnable = true },
		});

	if (!m_pipelineInDepth.has_value()) return false;

	return true;
}
//=============================================================================