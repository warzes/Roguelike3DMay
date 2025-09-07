#include "stdafx.h"
#include "TempPass.h"
#include "Uniforms.h"
#include "DirectionalLight.h"

//минимальный форвард шейдер
//timberborn

//=============================================================================
namespace
{
	const char* shaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec3 vertexNormal;
layout(location = 3) in vec2 vertexTexCoord;
layout(location = 4) in vec3 vertexTangent;

layout(binding = 0, std140) uniform SceneBlock {
	mat4 viewMatrix;
	mat4 projectionMatrix;
	vec3 cameraPosition;
};

layout(binding = 1, std140) uniform ModelMatricesBlock {
	mat4 modelMatrix;
};

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec4 fragWorldPosition;
layout(location = 3) out vec3 fragCameraPosition;

void main()
{
	fragTexCoord       = vertexTexCoord;
	fragNormal         = mat3(transpose(inverse(modelMatrix))) * vertexNormal;
	fragWorldPosition  = modelMatrix * vec4(vertexPosition, 1.0);
	fragCameraPosition = cameraPosition;

	gl_Position = projectionMatrix * viewMatrix * fragWorldPosition;
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec4 fragWorldPosition;
layout(location = 3) in vec3 fragCameraPosition;

#define MAX_POINT_LIGHTS 4

struct DirectionalLight {
	vec3 direction;
	vec3 color;
	float intensity;
	mat4 shadowMatrix;
};

struct PointLight {
	vec3 position;
	vec3 color;
	float intensity;
	float radius;
};

layout(binding = 2, std140) uniform DirectionalLightBlock {
	DirectionalLight dirLight;
	PointLight pointLight[MAX_POINT_LIGHTS];
	int pointLightCount;
};

layout(binding = 0) uniform sampler2D diffuseTexture;
layout(binding = 10) uniform sampler2D depthTexture;

layout(location = 0) out vec4 outputColor;

const float alphaTestThreshold = 0.1f;
const vec4 MaterialSpecularColor = vec4(0.1f, 0.1f, 0.1f, 1.0f); // TODO:
const float specularIntensity = 0.1f; // TODO:

vec3 computeDirectionalLightContribution(vec3 normal, vec3 lightDir, vec3 viewDir)
{
	vec3 halfwayDir = normalize(viewDir + lightDir);
	float diffuseImpact = max(dot(normal, lightDir), 0.0f);
	vec3 diffuse = diffuseImpact * dirLight.color * dirLight.intensity;

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0f), MaterialSpecularColor.w);
	vec3 specular = specularIntensity * spec * MaterialSpecularColor.rgb * dirLight.intensity;

	return diffuse + specular;
}

vec3 computePointLight(PointLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragWorldPosition.xyz);
	vec3 halfwayDir = normalize(viewDir + lightDir);
	float diffuseImpact = max(dot(normal, lightDir), 0.0f);
	vec3 diffuse = diffuseImpact * light.color * light.intensity;

	float distance = length(light.position - fragWorldPosition.xyz);
	//float attenuation = 1.0f / (
return vec3(0.0f);
}

// Проверка тени (PCF-like)
float computeShadow(vec4 shadowCoord, vec3 normal, vec3 lightDir)
{
	vec3 projCoords = shadowCoord.xyz / shadowCoord.w;
	projCoords = projCoords * 0.5 + 0.5; // [-1,1] => [0,1]

	if (projCoords.z > 1.0) return 0.0;

	float currentDepth = projCoords.z;

	float bias = max(0.002f * (1.0f - dot(normal, lightDir)), 0.0005f);

	// Простой PCF (3x3)
	vec2 texelSize = 1.0f / vec2(textureSize(depthTexture, 0));
	float pcfShadow = 0.0f;
	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			float closest = texture(depthTexture, projCoords.xy + vec2(x, y) * texelSize).r;
			pcfShadow += currentDepth > (closest + bias)  ? 1.0f : 0.0f;
		}
	}
	pcfShadow /= 9.0;
	return pcfShadow;
}

void main()
{
	vec4 albedo = texture(diffuseTexture, fragTexCoord);
	if (albedo.a <= alphaTestThreshold) discard;

	vec4 baseColor = albedo/* * MaterialDiffuseColor */;

	vec3 normal = normalize(fragNormal);
	vec3 viewDir = normalize(fragCameraPosition - fragWorldPosition.xyz);

	const float ambientStrength = 0.1f;
	vec3 ambient = ambientStrength * baseColor.rgb;
	
	vec3 lighting = vec3(0.0f);
	
	// Directional Light
	{
		vec4 shadowCoord = dirLight.shadowMatrix * fragWorldPosition;

		const vec3 lightDir = normalize(-dirLight.direction);
		const float shadow = computeShadow(shadowCoord, normal, lightDir);
		lighting += (ambient + (1.0f - shadow) * computeDirectionalLightContribution(normal, lightDir, viewDir));
	}

	// Point Light
	for (int i = 0; i < pointLightCount; ++i)
	{
		lighting += ambient + computePointLight(pointLight[i], normal, viewDir);
	}

	// Spot Light

	// Final
	outputColor.rgb = baseColor.rgb * lighting;
	outputColor.a = baseColor.a;
}
)";
}
//=============================================================================
bool TempPass::Init()
{
	DirectionalLightDataUBO.Init();

	gl::SamplerState sampleDesc;
	sampleDesc.minFilter = gl::MinFilter::Nearest;
	sampleDesc.magFilter = gl::MagFilter::Nearest;
	sampleDesc.addressModeU = gl::AddressMode::ClampToBorder;
	sampleDesc.addressModeV = gl::AddressMode::ClampToBorder;
	m_depthSampler = gl::Sampler(sampleDesc);
	

	m_rt.Init(GetWindowWidth(), GetWindowHeight(),
		RTAttachment{ gl::Format::R8G8B8A8_SRGB, "ShadowMapPassColor", gl::AttachmentLoadOp::Clear },
		RTAttachment{ gl::Format::D32_FLOAT, "ShadowMapPassDepth", gl::AttachmentLoadOp::Clear });

	auto vertexShader = gl::Shader(gl::ShaderType::VertexShader, shaderCodeVertex, "TempVS");
	auto fragmentShader = gl::Shader(gl::ShaderType::FragmentShader, shaderCodeFragment, "TempFS");

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
		 .name = "TempPipeline",
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
void TempPass::Close()
{
	m_depthSampler = {};
	DirectionalLightDataUBO.Close();
	m_rt.Close();
	m_pipeline = std::nullopt;
}
//=============================================================================
void TempPass::Begin(const glm::vec3& clearColor, const gl::Texture* depthTexture)
{
	DirectionalLightDataUBO->dirLight.color = gDirectionalLight.color;
	DirectionalLightDataUBO->dirLight.shadowMatrix = gDirectionalLight.GetMatrix();
	DirectionalLightDataUBO->dirLight.direction = gDirectionalLight.GetDirectional();
	DirectionalLightDataUBO->dirLight.intensity = gDirectionalLight.intensity;

	DirectionalLightDataUBO.Update();

	m_rt.Begin(clearColor);
	gl::Cmd::BindGraphicsPipeline(*m_pipeline);
	DirectionalLightDataUBO.Bind(2);
	gl::Cmd::BindSampledImage(10, *depthTexture, *m_depthSampler);
}
//=============================================================================
void TempPass::End()
{
	m_rt.End();
}
//=============================================================================