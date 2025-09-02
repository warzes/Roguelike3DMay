#include "stdafx.h"
#include "ShadowMapPass.h"
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
};

layout(binding = 1, std140) uniform ModelMatricesBlock {
	mat4 modelMatrix;
};

layout(location = 0) out vec4 worldPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;

void main()
{
	worldPosition = modelMatrix * vec4(vertexPosition, 1.0);
	gl_Position = projectionMatrix * viewMatrix * worldPosition;

	// Output all out variables
	fragNormal   = mat3(transpose(inverse(modelMatrix))) * vertexNormal;
	fragTexCoord = vertexTexCoord;
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

layout(location = 0) in vec4 worldPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

layout(binding = 2, std140) uniform SMFragmentBlock {
	vec4 albedoScaler;
	mat4 viewMat;
	vec2 depthClampPara;
	vec3 emissionColor;
	float alphaTestThreshold;
	bool depthOnly;
};

layout(binding = 0) uniform sampler2D albedoMap;

layout(location = 0) out float outputDepthMap;
layout(location = 1) out vec3 outputVplPosColor;
layout(location = 2) out vec3 outputVplAlbedoColor;
layout(location = 3) out vec3 outputVplNormalColor;

void main()
{
	vec4 albedo = texture(albedoMap, fragTexCoord);
	if (albedo.a <= alphaTestThreshold) discard;
	albedo.rgb = pow(albedo.rgb, vec3(2.2f));
	albedo.rgb *= albedoScaler.rgb;

	float d = -(viewMat * worldPosition).z;
	d = d * depthClampPara.y;
	d = clamp(d, 0.0f, 1.0f);
	outputDepthMap = d;
	if (!depthOnly)
	{
		outputVplPosColor = worldPosition.xyz;
		outputVplAlbedoColor = albedo.xyz + emissionColor.rgb;
		outputVplNormalColor = fragNormal;
	}
}
)";
}
//=============================================================================
bool ShadowMapPass::Init()
{
	m_rt.SetName("ShadowMapPassColor", "ShadowMapPassDepth");
	m_rt.SetSize(1024, 1024);

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
void ShadowMapPass::Close()
{
	m_rt.Close();
	m_pipeline = std::nullopt;
}
//=============================================================================
void ShadowMapPass::Begin()
{
	m_rt.Begin({ 0.0f, 0.0f, 0.0f });
	gl::Cmd::BindGraphicsPipeline(*m_pipeline);
}
//=============================================================================
void ShadowMapPass::End()
{
	m_rt.End();
}
//=============================================================================