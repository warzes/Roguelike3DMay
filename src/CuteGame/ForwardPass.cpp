#include "stdafx.h"
#include "ForwardPass.h"
#include "Uniforms.h"
#include "DirectionalLight.h"
//=============================================================================
bool ForwardPass::Init()
{
	LightDataUBO.Init();

	gl::SamplerState sampleDesc;
	sampleDesc.minFilter = gl::MinFilter::Nearest;
	sampleDesc.magFilter = gl::MagFilter::Nearest;
	sampleDesc.addressModeU = gl::AddressMode::ClampToBorder;
	sampleDesc.addressModeV = gl::AddressMode::ClampToBorder;
	m_depthSampler = gl::Sampler(sampleDesc);
	

	m_rt.Init(GetWindowWidth(), GetWindowHeight(),
		RTAttachment{ gl::Format::R8G8B8A8_SRGB, "ShadowMapPassColor", gl::AttachmentLoadOp::Clear },
		RTAttachment{ gl::Format::D32_FLOAT, "ShadowMapPassDepth", gl::AttachmentLoadOp::Clear });

	auto vertexShader = gl::Shader(gl::ShaderType::VertexShader, io::ReadShaderCode("CuteGameData/shaders/lighting.vert"), "lightingVS");
	auto fragmentShader = gl::Shader(gl::ShaderType::FragmentShader, io::ReadShaderCode("CuteGameData/shaders/lighting.frag"), "lightingFS");

	gl::ColorBlendState blendState;
	blendState.attachments.push_back({});
	blendState.attachments[0].blendEnable = true;
	blendState.attachments[0].srcColorBlendFactor = gl::BlendFactor::SrcAlpha;
	blendState.attachments[0].dstColorBlendFactor = gl::BlendFactor::OneMinusSrcAlpha;
	blendState.attachments[0].colorBlendOp = gl::BlendOp::Add;
	blendState.attachments[0].srcAlphaBlendFactor = gl::BlendFactor::SrcAlpha;
	blendState.attachments[0].dstAlphaBlendFactor = gl::BlendFactor::OneMinusSrcAlpha;
	blendState.attachments[0].alphaBlendOp = gl::BlendOp::Add;

	m_pipeline = gl::GraphicsPipeline({
		 .name = "ForwardPipeline",
		.vertexShader = &vertexShader,
		.fragmentShader = &fragmentShader,
		.inputAssemblyState = {.topology = gl::PrimitiveTopology::TriangleList },
		.vertexInputState = { MeshVertexInputBindingDesc },
		.depthState = {.depthTestEnable = true },
		.colorBlendState = blendState
	});
	return true;
}
//=============================================================================
void ForwardPass::Close()
{
	m_depthSampler = {};
	LightDataUBO.Close();
	m_rt.Close();
	m_pipeline = std::nullopt;
}
//=============================================================================
glm::vec3 getAttenuation(const float radius) noexcept // TODO:
{
	constexpr float constant{ 1.0f };
	const float linear{ 4.5f / radius };
	const float quadratic{ 75.0f / (radius * radius) };

	return glm::vec3(constant, linear, quadratic);
}
//=============================================================================
glm::vec2 updateCutoffAngles(const float _innerCutoffAngle, const float _outerCutoffAngle) noexcept // TODO:
{
	return glm::vec2(glm::cos(glm::radians(_innerCutoffAngle)), glm::cos(glm::radians(_outerCutoffAngle)));
}
//=============================================================================
void ForwardPass::Begin(const glm::vec3& clearColor, const gl::Texture* depthTexture)
{
	LightDataUBO->dirLight.color = gDirectionalLight.GetColor();
	LightDataUBO->dirLight.lightSpaceMatrix = gDirectionalLight.GetMatrix();
	LightDataUBO->dirLight.direction = gDirectionalLight.GetDirectional();
	LightDataUBO->dirLight.intensity = gDirectionalLight.GetIntensity();

	LightDataUBO->pointLights[0].position = glm::vec3(-2.0f, 0.5f, -1.2f);
	LightDataUBO->pointLights[0].color = glm::vec3(1.0f, 0.2f, 0.2f);
	LightDataUBO->pointLights[0].intensity = 30.0f;
	LightDataUBO->pointLights[0].attenuation = getAttenuation(50.0f);

	LightDataUBO->pointLights[1].position = glm::vec3(2.0f, 1.0f, -1.2f);
	LightDataUBO->pointLights[1].color = glm::vec3(0.2f, 1.0f, 0.2f);
	LightDataUBO->pointLights[1].intensity = 5.0f;
	LightDataUBO->pointLights[1].attenuation = getAttenuation(20.0f);

	LightDataUBO->pointLights[2].position = glm::vec3(-2.0f, 0.5f, 4.2f);
	LightDataUBO->pointLights[2].color = glm::vec3(0.2f, 0.2f, 1.0f);
	LightDataUBO->pointLights[2].intensity = 3.0f;
	LightDataUBO->pointLights[2].attenuation = getAttenuation(15.0f);

	LightDataUBO->pointLights[3].position = glm::vec3(-4.0f, 0.5f, 3.2f);
	LightDataUBO->pointLights[3].color = glm::vec3(0.2f, 1.0f, 1.0f);
	LightDataUBO->pointLights[3].intensity = 3.0f;
	LightDataUBO->pointLights[3].attenuation = getAttenuation(15.0f);

	LightDataUBO->spotLights[0].position = glm::vec3(-4.0f, 3.0f, 4.0f);
	LightDataUBO->spotLights[0].direction = glm::vec3(0.0f, -1.0f, 0.0f);
	LightDataUBO->spotLights[0].color = glm::vec3(0.0f, 0.0f, 1.0f);
	LightDataUBO->spotLights[0].intensity = 10.0f;
	LightDataUBO->spotLights[0].attenuation = getAttenuation(10.0f);
	LightDataUBO->spotLights[0].angles = updateCutoffAngles(12.5f, 17.5f);

	LightDataUBO.Update();

	m_rt.Begin(clearColor);
	gl::Cmd::BindGraphicsPipeline(*m_pipeline);
	LightDataUBO.Bind(2);
	gl::Cmd::BindSampledImage(10, *depthTexture, *m_depthSampler);
}
//=============================================================================
void ForwardPass::End()
{
	m_rt.End();
}
//=============================================================================