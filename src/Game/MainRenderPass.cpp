#include "stdafx.h"
#include "MainRenderPass.h"
#include "GameModel.h"
//=============================================================================
bool MainRenderPass::Init()
{
	if (!createPipeline())
		return false;

	m_globalUbo   = gl4::TypedBuffer<GlobalUniforms>(gl4::BufferStorageFlag::DynamicStorage);
	m_objectUbo   = gl4::TypedBuffer<ObjectUniforms>(gl4::BufferStorageFlag::DynamicStorage);
	m_materialUbo = gl4::TypedBuffer<MaterialUniforms>(gl4::BufferStorageFlag::DynamicStorage);

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

	return true;
}
//=============================================================================
void MainRenderPass::Close()
{
	m_globalUbo = {};
	m_objectUbo = {};
	m_materialUbo = {};
	m_nearestSampler = {};
	m_linearSampler = {};
	m_pipeline = {};
}
//=============================================================================
void MainRenderPass::Begin(Camera& cam)
{
	m_globalUboData.view = cam.GetViewMatrix();
	m_globalUboData.proj = glm::perspective(glm::radians(65.0f), GetWindowAspect(), 0.01f, 1000.0f);
	m_globalUbo->UpdateData(m_globalUboData);

	gl4::Cmd::BindGraphicsPipeline(m_pipeline.value());
	gl4::Cmd::BindUniformBuffer(0, m_globalUbo.value());
}
//=============================================================================
void MainRenderPass::DrawModel(GameModel& model)
{
	const gl4::Sampler& sampler = (model.textureFilter == gl4::MagFilter::Linear) 
		? m_linearSampler.value() 
		: m_nearestSampler.value();
		
	m_objectUboData.model = model.GetModelMat();
	m_objectUbo->UpdateData(m_objectUboData);
	gl4::Cmd::BindUniformBuffer(1, m_objectUbo.value());

	m_materialUboData.hasDiffuseTexture   = model.material.diffuseTexture  != nullptr;
	m_materialUboData.hasSpecularTexture  = model.material.specularTexture != nullptr;
	m_materialUboData.hasEmissionTexture  = model.material.emissionTexture != nullptr;
	m_materialUboData.hasNormalMapTexture = model.material.normalTexture   != nullptr;
	m_materialUboData.hasDepthMapTexture  = model.material.depthTexture    != nullptr;
	m_materialUbo->UpdateData(m_materialUboData);
	gl4::Cmd::BindUniformBuffer(2, m_materialUbo.value());

	if (model.material.diffuseTexture)
		gl4::Cmd::BindSampledImage(0, *model.material.diffuseTexture, sampler);
	if (model.material.specularTexture)
		gl4::Cmd::BindSampledImage(1, *model.material.specularTexture, sampler);
	if (model.material.emissionTexture)
		gl4::Cmd::BindSampledImage(2, *model.material.emissionTexture, sampler);
	if (model.material.normalTexture)
		gl4::Cmd::BindSampledImage(3, *model.material.normalTexture, sampler);
	if (model.material.depthTexture)
		gl4::Cmd::BindSampledImage(4, *model.material.depthTexture, sampler);

	model.mesh->Bind();
}
//=============================================================================
bool MainRenderPass::createPipeline()
{
	auto vertexShader = gl4::Shader(gl4::PipelineStage::VertexShader, io::ReadShaderCode("GameData/shaders/MainShader.vert"), "MainShader VS");
	if (!vertexShader.IsValid()) return false;
	auto fragmentShader = gl4::Shader(gl4::PipelineStage::FragmentShader, io::ReadShaderCode("GameData/shaders/MainShader.frag"), "MainShader FS");
	if (!fragmentShader.IsValid()) return false;

	m_pipeline = gl4::GraphicsPipeline({
		.name               = "Model Pipeline",
		.vertexShader       = &vertexShader,
		.fragmentShader     = &fragmentShader,
		.inputAssemblyState = {.topology = gl4::PrimitiveTopology::TRIANGLE_LIST},
		.vertexInputState   = {MeshVertexInputBindingDescs},
		.depthState         = {.depthTestEnable = true, .depthWriteEnable = true},
		});
	if (!m_pipeline.has_value()) return false;

	return true;
}
//=============================================================================