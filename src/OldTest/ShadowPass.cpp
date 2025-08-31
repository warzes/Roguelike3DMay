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

	gl::SamplerState sampleDesc;
	sampleDesc.anisotropy   = gl::SampleCount::Samples1;
	sampleDesc.minFilter    = gl::MinFilter::Linear;
	sampleDesc.magFilter    = gl::MagFilter::Linear;
	sampleDesc.addressModeU = gl::AddressMode::ClampToEdge;
	sampleDesc.addressModeV = gl::AddressMode::ClampToEdge;
	m_linearSampler = gl::Sampler(sampleDesc);

	m_ubo = gl::TypedBuffer<ShadowUniforms>(gl::BufferStorageFlag::DynamicStorage);

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
	auto depthAttachment = gl::RenderDepthStencilAttachment{
	  .texture = *shadow.depthTexture,
	  .loadOp = gl::AttachmentLoadOp::Clear,
	  .clearValue = {.depth = 1.0f},
	};
	gl::Viewport view{ .drawRect = {{0, 0}, {shadow.width, shadow.height}} };
	gl::BeginRendering({ .viewport = view, .depthAttachment = depthAttachment });
	gl::Cmd::BindGraphicsPipeline(m_pipeline.value());
	m_uboData.vp = shadow.lightProjection * shadow.lightView;
}
//=============================================================================
void ShadowPass::DrawModel(GameModelOld& model)
{
	m_uboData.model = model.GetModelMat();
	m_ubo->UpdateData(m_uboData);
	gl::Cmd::BindUniformBuffer(0, m_ubo.value());
	model.mesh->Bind(std::nullopt);
}
//=============================================================================
void ShadowPass::End()
{
	gl::EndRendering();
}
//=============================================================================
bool ShadowPass::createPipeline()
{
	auto vertexShader = gl::Shader(gl::ShaderType::VertexShader, io::ReadShaderCode("GameData/shaders/ShadowPass.vert"), "ShadowPass VS");
	if (!vertexShader.IsValid()) return false;
	auto fragmentShader = gl::Shader(gl::ShaderType::FragmentShader, io::ReadShaderCode("GameData/shaders/ShadowPass.frag"), "ShadowPass FS");
	if (!fragmentShader.IsValid()) return false;

	m_pipeline = gl::GraphicsPipeline({
		.name = "Model In Depth Pipeline",
		.vertexShader = &vertexShader,
		.fragmentShader = &fragmentShader,
		.inputAssemblyState = {.topology = gl::PrimitiveTopology::TriangleList},
		.vertexInputState = {MeshVertexInputBindingDescs},
		.depthState = {.depthTestEnable = true, .depthWriteEnable = true},
		});
	if (!m_pipeline.has_value()) return false;

	return true;
}
//=============================================================================