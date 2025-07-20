#include "stdafx.h"
#include "ShadowPass.h"
#include "GameModel.h"
//=============================================================================
bool ShadowPass::Init()
{
	if (!createPipeline())
		return false;

	gl4::SamplerState sampleDesc;
	sampleDesc.anisotropy = gl4::SampleCount::Samples1;
	sampleDesc.minFilter = gl4::MinFilter::Linear;
	sampleDesc.magFilter = gl4::MagFilter::Linear;
	sampleDesc.addressModeU = gl4::AddressMode::ClampToEdge;
	sampleDesc.addressModeV = gl4::AddressMode::ClampToEdge;
	m_linearSampler = gl4::Sampler(sampleDesc);

	// TODO: переделать чтобы юбо был общий для всех
	m_globalUbo = gl4::TypedBuffer<GlobalUniforms>(gl4::BufferStorageFlag::DynamicStorage);
	m_objectUbo = gl4::TypedBuffer<ObjectUniforms>(gl4::BufferStorageFlag::DynamicStorage);

	gl4::TextureCreateInfo createInfo{
		.imageType = gl4::ImageType::Tex2D,
		.format = gl4::Format::D32_FLOAT,
		.extent = { width, height, 1},
		.mipLevels = 1,
		.arrayLayers = 1,
		.sampleCount = gl4::SampleCount::Samples1,
	};
	depthTexture = new gl4::Texture(createInfo, "ShadowDepth");

	rtAttachment = new gl4::RenderDepthStencilAttachment{
		.texture = *depthTexture,
		.loadOp = gl4::AttachmentLoadOp::Clear,
		.clearValue = {.depth = 1.0f},
	};

	gl4::Viewport view{ .drawRect = {{0, 0}, {width, height}} };
	viewport = new gl4::RenderInfo({ .viewport = view, .depthAttachment = *rtAttachment });

	shadowLightPos = { 2.0f, 2.0f, 1.0f };

	// Shaders
	lightProjection = glm::ortho(-10.f, 10.f, -10.f, 10.f, -20.0f, 20.0f);
	//lightView = glm::lookAt(glm::normalize(-shadowLightPos), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1, 0, 0));
	lightView = glm::lookAt(glm::normalize(shadowLightPos), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0, 1, 0));

	return true;
}
//=============================================================================
void ShadowPass::Close()
{
	delete depthTexture;
	delete rtAttachment;
	delete viewport;

	m_linearSampler = {};
	m_globalUbo = {};
	m_objectUbo = {};
	m_pipeline = {};
}
//=============================================================================
void ShadowPass::Begin()
{
	gl4::BeginRendering(*viewport);

	m_globalUboData.view = lightView;
	m_globalUboData.proj = lightProjection;
	m_globalUboData.eyePosition = shadowLightPos;
	m_globalUbo->UpdateData(m_globalUboData);

	gl4::Cmd::BindGraphicsPipeline(m_pipeline.value());
	gl4::Cmd::BindUniformBuffer(0, m_globalUbo.value());
}
//=============================================================================
void ShadowPass::DrawModel(GameModel& model)
{
	m_objectUboData.model = model.GetModelMat();
	m_objectUbo->UpdateData(m_objectUboData);
	gl4::Cmd::BindUniformBuffer(1, m_objectUbo.value());

	model.mesh->Bind();
}
//=============================================================================
void ShadowPass::End()
{
	gl4::EndRendering();
}
//=============================================================================
void ShadowPass::BindShadowMap(uint32_t index)
{
	gl4::Cmd::BindSampledImage(index, *depthTexture, m_linearSampler.value());
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