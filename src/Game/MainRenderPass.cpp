#include "stdafx.h"
#include "MainRenderPass.h"
#include "GameModel.h"
#include "World.h"
#define DEFAULT_DIRECTIONAL_LIGHT_SHADOW_MAP_BIAS 0.005f
#define DEFAULT_POINT_LIGHT_SHADOW_MAP_BIAS 0.0075f
//=============================================================================
inline std::optional<gl::Texture> createPoissonDiscDistribution(size_t numSamples)
{
	auto defaultPRNG = PoissonGenerator::DefaultPRNG();
	auto points = PoissonGenerator::GeneratePoissonPoints(numSamples * 2, defaultPRNG);
	size_t attempts = 0;
	while (points.size() < numSamples && ++attempts < 100)
	{
		auto defaultPRNG2 = PoissonGenerator::DefaultPRNG();
		points = PoissonGenerator::GeneratePoissonPoints(numSamples * 2, defaultPRNG2);
	}

	if (attempts == 100)
	{
		std::cout << "couldn't generate Poisson-disc distribution with " << numSamples << " samples" << std::endl;
		numSamples = points.size();
	}
	std::vector<float> data(numSamples * 2);
	for (auto i = 0, j = 0; i < numSamples; i++, j += 2)
	{
		auto& point = points[i];
		data[j] = point.x;
		data[j + 1] = point.y;
	}

	const gl::TextureCreateInfo createInfo{
		  .imageType = gl::ImageType::Tex1D,
		  .format = gl::Format::R32G32_FLOAT,
		  .extent = {static_cast<uint32_t>(numSamples), 1, 1},
		  .mipLevels = 1,
		  .arrayLayers = 1,
		  .sampleCount = gl::SampleCount::Samples1,
	};
	std::optional<gl::Texture> texture = gl::Texture(createInfo);

	texture.value().UpdateImage({
	  .extent = createInfo.extent,
	  .format = gl::UploadFormat::RG,
	  .type = gl::UploadType::FLOAT,
	  .pixels = &data[0],
		});

	return texture;
}
//=============================================================================
bool MainRenderPass::Init(World* world)
{
	if (!createPipeline())
		return false;

	m_world = world;

	m_globalUbo   = gl::TypedBuffer<GlobalUniforms>(gl::BufferStorageFlag::DynamicStorage);
	m_objectUbo   = gl::TypedBuffer<ObjectUniforms>(gl::BufferStorageFlag::DynamicStorage);
	m_materialUbo = gl::TypedBuffer<MaterialUniforms>(gl::BufferStorageFlag::DynamicStorage);
	m_mainFragUbo = gl::TypedBuffer<MainFragmentUniforms>(gl::BufferStorageFlag::DynamicStorage);

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

	m_lightSSBO.emplace(std::span(m_world->GetLights()), gl::BufferStorageFlag::DynamicStorage);

	m_distributions0 = createPoissonDiscDistribution(m_numBlockerSearchSamples);
	m_distributions1 = createPoissonDiscDistribution(m_numPCFSamples);

	sampleDesc.minFilter = gl::MinFilter::Nearest;
	sampleDesc.magFilter = gl::MagFilter::Nearest;
	sampleDesc.addressModeU = gl::AddressMode::ClampToEdge;
	sampleDesc.addressModeV = gl::AddressMode::ClampToEdge;
	m_distributionsSampler = gl::Sampler(sampleDesc);

	return true;
}
//=============================================================================
void MainRenderPass::Close()
{
	m_distributions0 = {};
	m_distributions1 = {};
	m_distributionsSampler = {};
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

	gl::Cmd::BindGraphicsPipeline(m_pipeline.value());
	gl::Cmd::BindUniformBuffer(0, m_globalUbo.value());
	gl::Cmd::BindStorageBuffer(0, *m_lightSSBO);
}
//=============================================================================
void MainRenderPass::DrawModel(GameModelOld& model)
{
	const gl::Sampler& sampler = (model.textureFilter == gl::MagFilter::Linear) 
		? m_linearSampler.value() 
		: m_nearestSampler.value();
		
	m_objectUboData.model = model.GetModelMat();
	m_objectUbo->UpdateData(m_objectUboData);
	gl::Cmd::BindUniformBuffer(1, m_objectUbo.value());

	m_materialUboData.diffuse             = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	m_materialUboData.hasDiffuseTexture   = model.material.diffuseTexture  != nullptr;
	m_materialUboData.hasSpecularTexture  = model.material.specularTexture != nullptr;
	m_materialUboData.hasEmissionTexture  = model.material.emissionTexture != nullptr;
	m_materialUboData.hasNormalMapTexture = model.material.normalTexture   != nullptr;
	m_materialUboData.hasDepthMapTexture  = model.material.depthTexture    != nullptr;
	m_materialUboData.noLighing           = model.material.noLighing;
	m_materialUbo->UpdateData(m_materialUboData);
	gl::Cmd::BindUniformBuffer(2, m_materialUbo.value());

	m_mainFragUboData.invView = glm::inverse(m_globalUboData.view);
	m_mainFragUboData.lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, 10.0f);
	m_mainFragUboData.directionalLightShadowMapBias = DEFAULT_DIRECTIONAL_LIGHT_SHADOW_MAP_BIAS;
	m_mainFragUboData.pointLightShadowMapBias = DEFAULT_POINT_LIGHT_SHADOW_MAP_BIAS;

	m_mainFragUboData.shadowMapViewProjection0 = m_world->GetShadowMap()[0].lightProjection * m_world->GetShadowMap()[0].lightView;

	// TODO:
#define NEAR 0.1f
#define FOV 60.0f
	m_mainFragUboData.frustumSize = 2 * NEAR * std::tanf(FOV * 0.5f) * GetWindowAspect(); // TODO: это должно делаться в ресайзе окна

	m_mainFragUboData.MaxNumLightSources = m_world->GetLights().size();
	m_mainFragUbo->UpdateData(m_mainFragUboData);
	gl::Cmd::BindUniformBuffer(3, m_mainFragUbo.value());

	if (model.material.diffuseTexture)
		gl::Cmd::BindSampledImage(0, *model.material.diffuseTexture, sampler);
	if (model.material.specularTexture)
		gl::Cmd::BindSampledImage(1, *model.material.specularTexture, sampler);
	if (model.material.emissionTexture)
		gl::Cmd::BindSampledImage(2, *model.material.emissionTexture, sampler);
	if (model.material.normalTexture)
		gl::Cmd::BindSampledImage(3, *model.material.normalTexture, sampler);
	if (model.material.depthTexture)
		gl::Cmd::BindSampledImage(4, *model.material.depthTexture, sampler);

	gl::Cmd::BindSampledImage(5, *m_distributions0, *m_distributionsSampler);
	gl::Cmd::BindSampledImage(6, *m_distributions1, *m_distributionsSampler);

	m_world->GetShadowMap()[0].Bind(7, sampler); // TODO: сеплер для тени

	model.mesh->Bind();
}
//=============================================================================
bool MainRenderPass::createPipeline()
{
	auto vertexShader = gl::Shader(gl::ShaderType::VertexShader, io::ReadShaderCode("GameData/shaders/MainShader.vert"), "MainShader VS");
	if (!vertexShader.IsValid()) return false;
	auto fragmentShader = gl::Shader(gl::ShaderType::FragmentShader, io::ReadShaderCode("GameData/shaders/MainShader.frag"), "MainShader FS");
	if (!fragmentShader.IsValid()) return false;

	m_pipeline = gl::GraphicsPipeline({
		.name               = "Model Pipeline",
		.vertexShader       = &vertexShader,
		.fragmentShader     = &fragmentShader,
		.inputAssemblyState = {.topology = gl::PrimitiveTopology::TriangleList},
		.vertexInputState   = {MeshVertexInputBindingDescs},
		.depthState         = {.depthTestEnable = true, .depthWriteEnable = true},
		});
	if (!m_pipeline.has_value()) return false;

	return true;
}
//=============================================================================