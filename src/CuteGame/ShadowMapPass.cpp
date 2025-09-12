#include "stdafx.h"
#include "ShadowMapPass.h"
#include "Uniforms.h"
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

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec4 fragWorldPosition;

void main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);

	// Output all out variables
	fragTexCoord      = vertexTexCoord;
	fragNormal        = mat3(transpose(inverse(modelMatrix))) * vertexNormal;
	fragWorldPosition = modelMatrix * vec4(vertexPosition, 1.0);
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec4 fragWorldPosition;

layout(binding = 2, std140) uniform SMFragmentBlock {
	mat4 viewMat;
	vec3 albedoScaler;
	vec2 depthClampPara;
	vec3 emissionColor;
	float alphaTestThreshold;
	bool depthOnly;
};

layout(binding = 0) uniform sampler2D albedoMap;

layout(location = 0) out float outputDepthMap;
layout(location = 1) out vec3 outputPosColor;
layout(location = 2) out vec3 outputAlbedoColor;
layout(location = 3) out vec3 outputNormalColor;

void main()
{
	vec4 albedo = texture(albedoMap, fragTexCoord);
	if (albedo.a <= alphaTestThreshold) discard;
	albedo.rgb *= albedoScaler;

	float d = -(viewMat * fragWorldPosition).z;
	d = d * depthClampPara.y;
	d = clamp(d, 0.0f, 1.0f);
	outputDepthMap = d;
	if (!depthOnly)
	{
		outputPosColor = fragWorldPosition.xyz;
		outputAlbedoColor = albedo.xyz + emissionColor.rgb;
		outputNormalColor = fragNormal;
	}
}
)";
}
//=============================================================================
bool Temp2Pass::Init()
{
	std::vector<RTAttachment> rts = {
		RTAttachment{ gl::Format::R8_UNORM, "ShadowMapDepthMap", gl::AttachmentLoadOp::Clear }, // TODO: DontCare?
		RTAttachment{ gl::Format::R8G8B8_UNORM, "ShadowMapPosColor", gl::AttachmentLoadOp::Clear }, // TODO: DontCare?
		RTAttachment{ gl::Format::R8G8B8_SRGB, "ShadowMapAlbedoColor", gl::AttachmentLoadOp::Clear }, // TODO: DontCare?
		RTAttachment{ gl::Format::R8G8B8_UNORM, "ShadowMapNormalColor", gl::AttachmentLoadOp::Clear }, // TODO: DontCare?
	};
	m_rt.Init(GetWindowWidth(), GetWindowHeight(), rts, RTDAttachment{ gl::Format::D32_FLOAT, "ShadowMapPassDepth", gl::AttachmentLoadOp::Clear });

	auto vertexShader = gl::Shader(gl::ShaderType::VertexShader, shaderCodeVertex, "ShadowMapPassVS");
	auto fragmentShader = gl::Shader(gl::ShaderType::FragmentShader, shaderCodeFragment, "ShadowMapPassFS");

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
		 .name = "ShadowMapPassPipeline",
		.vertexShader = &vertexShader,
		.fragmentShader = &fragmentShader,
		.inputAssemblyState = {.topology = gl::PrimitiveTopology::TriangleList },
		.vertexInputState = { MeshVertexInputBindingDesc },
		.depthState = {.depthTestEnable = true },
		//.colorBlendState = blendState
		});

	return true;
}
//=============================================================================
void Temp2Pass::Close()
{
	m_rt.Close();
	m_pipeline = std::nullopt;
}
//=============================================================================
void Temp2Pass::Begin()
{
	SMFragmentDataUBO->viewMat = glm::mat4(1.0f);
	SMFragmentDataUBO->albedoScaler = { 1.0f, 1.0f, 1.0f };
	SMFragmentDataUBO->depthClampPara = { 0.1f, 1.0f / 1000.0f };
	SMFragmentDataUBO->emissionColor = { 0.0f, 0.0f, 0.0f };
	SMFragmentDataUBO->alphaTestThreshold = 0.2f;
	SMFragmentDataUBO->depthOnly = 0;

	m_rt.Begin({ 0.0f, 0.0f, 0.0f });
	gl::Cmd::BindGraphicsPipeline(*m_pipeline);
	SMFragmentDataUBO.Bind(2);
}
//=============================================================================
void Temp2Pass::End()
{
	m_rt.End();
}
//=============================================================================