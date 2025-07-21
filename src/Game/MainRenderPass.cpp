#include "stdafx.h"
#include "MainRenderPass.h"
#include "GameModel.h"
#include "World.h"
//=============================================================================
bool MainRenderPass::Init(World* world)
{
	if (!createPipeline())
		return false;

	m_world = world;

	m_globalUbo   = gl4::TypedBuffer<GlobalUniforms>(gl4::BufferStorageFlag::DynamicStorage);
	m_objectUbo   = gl4::TypedBuffer<ObjectUniforms>(gl4::BufferStorageFlag::DynamicStorage);
	m_materialUbo = gl4::TypedBuffer<MaterialUniforms>(gl4::BufferStorageFlag::DynamicStorage);
	m_mainFragUbo = gl4::TypedBuffer<MainFragmentUniforms>(gl4::BufferStorageFlag::DynamicStorage);

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

	m_lightSSBO.emplace(std::span(m_world->GetLights()), gl4::BufferStorageFlag::DynamicStorage);

	return true;
}
//=============================================================================
void MainRenderPass::Close()
{
	m_lightSSBO = {};
	m_globalUbo = {};
	m_objectUbo = {};
	m_mainFragUbo = {};
	m_materialUbo = {};
	m_nearestSampler = {};
	m_linearSampler = {};
	m_pipeline = {};
}
//=============================================================================
void MainRenderPass::Begin(Camera& cam, const glm::mat4& proj)
{
	m_lightSSBO->UpdateData(std::span(m_world->GetLights()));

	m_globalUboData.view = cam.GetViewMatrix();
	m_globalUboData.proj = proj;
	m_globalUboData.eyePosition = cam.Position;
	m_globalUbo->UpdateData(m_globalUboData);

	gl4::Cmd::BindGraphicsPipeline(m_pipeline.value());
	gl4::Cmd::BindUniformBuffer(0, m_globalUbo.value());
	gl4::Cmd::BindStorageBuffer(0, *m_lightSSBO);
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

	m_materialUboData.diffuse             = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	m_materialUboData.hasDiffuseTexture   = model.material.diffuseTexture  != nullptr;
	m_materialUboData.hasSpecularTexture  = model.material.specularTexture != nullptr;
	m_materialUboData.hasEmissionTexture  = model.material.emissionTexture != nullptr;
	m_materialUboData.hasNormalMapTexture = model.material.normalTexture   != nullptr;
	m_materialUboData.hasDepthMapTexture  = model.material.depthTexture    != nullptr;
	m_materialUboData.noLighing           = model.material.noLighing;
	m_materialUbo->UpdateData(m_materialUboData);
	gl4::Cmd::BindUniformBuffer(2, m_materialUbo.value());

	m_mainFragUboData.numLight = m_world->GetLights().size();
	m_mainFragUbo->UpdateData(m_mainFragUboData);
	gl4::Cmd::BindUniformBuffer(3, m_mainFragUbo.value());

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