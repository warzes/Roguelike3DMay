#include "stdafx.h"
#include "ShadowPass.h"
#include "GameModel.h"
#include "World.h"
//=============================================================================
bool ShadowPass::Init(World* world)
{
	if (!createPipeline())
		return false;

	m_world = world;

	gl4::SamplerState sampleDesc;
	sampleDesc.anisotropy   = gl4::SampleCount::Samples1;
	sampleDesc.minFilter    = gl4::MinFilter::Linear;
	sampleDesc.magFilter    = gl4::MagFilter::Linear;
	sampleDesc.addressModeU = gl4::AddressMode::ClampToEdge;
	sampleDesc.addressModeV = gl4::AddressMode::ClampToEdge;
	m_linearSampler = gl4::Sampler(sampleDesc);

	m_ubo = gl4::TypedBuffer<ShadowUniforms>(gl4::BufferStorageFlag::DynamicStorage);

	return true;
}
//=============================================================================
void ShadowPass::Close()
{
	m_linearSampler = {};
	m_ubo = {};
	m_pipeline = {};
}
//=============================================================================
void ShadowPass::Begin(const ShadowMap& shadow)
{
	auto depthAttachment = gl4::RenderDepthStencilAttachment{
	  .texture = *shadow.depthTexture,
	  .loadOp = gl4::AttachmentLoadOp::Clear,
	  .clearValue = {.depth = 1.0f},
	};
	gl4::Viewport view{ .drawRect = {{0, 0}, {shadow.width, shadow.height}} };
	gl4::BeginRendering({ .viewport = view, .depthAttachment = depthAttachment });
	gl4::Cmd::BindGraphicsPipeline(m_pipeline.value());
	m_uboData.vp = shadow.lightProjection * shadow.lightView;
}
//=============================================================================
void ShadowPass::DrawModel(GameModelOld& model)
{
	m_uboData.model = model.GetModelMat();
	m_ubo->UpdateData(m_uboData);
	gl4::Cmd::BindUniformBuffer(0, m_ubo.value());
	model.mesh->Bind();
}
//=============================================================================
void ShadowPass::End()
{
	gl4::EndRendering();
}
//=============================================================================
bool ShadowPass::createPipeline()
{
	auto vertexShader = gl4::Shader(gl4::PipelineStage::VertexShader, io::ReadShaderCode("GameData/shaders/ShadowPass.vert"), "ShadowPass VS");
	if (!vertexShader.IsValid()) return false;
	auto fragmentShader = gl4::Shader(gl4::PipelineStage::FragmentShader, io::ReadShaderCode("GameData/shaders/ShadowPass.frag"), "ShadowPass FS");
	if (!fragmentShader.IsValid()) return false;

	m_pipeline = gl4::GraphicsPipeline({
		.name = "Model In Depth Pipeline",
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