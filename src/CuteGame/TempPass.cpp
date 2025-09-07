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

void main()
{
	fragTexCoord      = vertexTexCoord;
	fragNormal        = mat3(transpose(inverse(modelMatrix))) * vertexNormal;
	fragWorldPosition = modelMatrix * vec4(vertexPosition, 1.0);

	gl_Position = projectionMatrix * viewMatrix * fragWorldPosition;
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec4 fragWorldPosition;

#define MAX_POINT_LIGHTS 4

struct DirectionalLight {
	vec3 direction;
	vec3 color;
	float intensity;
	mat4 shadowMatrix;
};

struct PointLight {
	vec3 position;
};

layout(binding = 2, std140) uniform DirectionalLightBlock {
	DirectionalLight dirLight;
	//PointLight pointLight[MAX_POINT_LIGHTS];
};

layout(binding = 0) uniform sampler2D diffuseTexture;
layout(binding = 1) uniform sampler2D depthTexture;

layout(location = 0) out vec4 outputColor;

const float alphaTestThreshold = 0.1f;
const vec3 ambientColor = vec3(0.05); // TODO:

float computeDirLightContribution(vec3 normal, vec3 lightDir)
{
	float NdotL = dot(normal, -lightDir);
	NdotL = max(NdotL, 0.0f);
	return NdotL;
}

// Проверка тени (PCF-like)
float computeShadow(vec4 shadowCoord) {
	vec3 projCoords = shadowCoord.xyz / shadowCoord.w;
	projCoords = projCoords * 0.5 + 0.5; // [-1,1] => [0,1]

	if (projCoords.z > 1.0) return 0.0;

	float closestDepth = texture(depthTexture, projCoords.xy).r;
	float currentDepth = projCoords.z;

	float bias = 0.005;
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	// Простой PCF (3x3)
	vec2 texelSize = 1.0 / vec2(textureSize(depthTexture, 0));
	float pcf = 0.0;
	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			float closest = texture(depthTexture, projCoords.xy + vec2(x, y) * texelSize).r;
			pcf += currentDepth - bias > closest ? 1.0 : 0.0;
		}
	}
	pcf /= 9.0;
	return pcf;
}


void main()
{
	vec4 albedo = texture(diffuseTexture, fragTexCoord);
	if (albedo.a <= alphaTestThreshold) discard;

	vec4 shadowCoord = dirLight.shadowMatrix * fragWorldPosition; // TODO: in vertex shader

	vec3 normal = normalize(fragNormal);
	
	outputColor.rgb = ambientColor * albedo.rgb;
	
	// Directional Light
	{
		vec3 lightDir = dirLight.direction;
		float diff = computeDirLightContribution(normal, lightDir);
		float shadow = computeShadow(shadowCoord);
		vec3 dirLightColor = dirLight.color * dirLight.intensity;
		outputColor.rgb += (1.0 - shadow) * diff * dirLightColor * albedo.rgb;
	}

	outputColor.a = albedo.a;
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
	gl::Cmd::BindSampledImage(1, *depthTexture, *m_depthSampler);
}
//=============================================================================
void TempPass::End()
{
	m_rt.End();
}
//=============================================================================