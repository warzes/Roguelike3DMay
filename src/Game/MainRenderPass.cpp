#include "stdafx.h"
#include "MainRenderPass.h"
#include "GameModel.h"
//=============================================================================
bool MainRenderPass::Init()
{
	if (!createPipeline())
		return false;

	m_globalUbo = gl4::TypedBuffer<GlobalUniforms>(gl4::BufferStorageFlag::DynamicStorage);
	m_objectUbo = gl4::TypedBuffer<ObjectUniforms>(gl4::BufferStorageFlag::DynamicStorage);

	gl4::SamplerState sampleDesc;
	sampleDesc.minFilter = gl4::MinFilter::Nearest;
	sampleDesc.magFilter = gl4::MagFilter::Nearest;
	sampleDesc.addressModeU = gl4::AddressMode::Repeat;
	sampleDesc.addressModeV = gl4::AddressMode::Repeat;
	m_nearestSampler = gl4::Sampler(sampleDesc);

	sampleDesc.anisotropy = gl4::SampleCount::Samples16;
	sampleDesc.minFilter = gl4::MinFilter::LinearMimapLinear;
	sampleDesc.magFilter = gl4::MagFilter::Linear;
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
	m_nearestSampler = {};
	m_linearSampler = {};
	m_pipeline = {};
}
//=============================================================================
void MainRenderPass::BeginFrame(Camera& cam)
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
	gl4::Cmd::BindSampledImage(0, *model.material.diffuseTexture, sampler);

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
		 .name = "Model Pipeline",
		.vertexShader = &vertexShader,
		.fragmentShader = &fragmentShader,
		.inputAssemblyState = {.topology = gl4::PrimitiveTopology::TRIANGLE_LIST},
		.vertexInputState = {MeshVertexInputBindingDescs},
		.depthState = {.depthTestEnable = true, .depthWriteEnable = true},
		});

	if (!m_pipeline.has_value()) return false;

	return true;
}
//=============================================================================