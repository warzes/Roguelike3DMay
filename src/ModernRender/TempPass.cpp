#include "stdafx.h"
#include "TempPass.h"
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

layout(binding = 0) uniform sampler2D diffuseTexture;

layout(location = 0) out vec4 outputColor;

const float alphaTestThreshold = 0.1f;

void main()
{
	vec4 albedo = texture(diffuseTexture, fragTexCoord);
	if (albedo.a <= alphaTestThreshold) discard;
	outputColor = albedo;

//outputColor = vec4(34.0f/255.0f, 177.0f/255.0f, 76.0f/255.0f, 1.0f);
}
)";
}
//=============================================================================
bool ForwardPass::Init()
{
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
void ForwardPass::Close()
{
	m_rt.Close();
	m_pipeline = std::nullopt;
}
//=============================================================================
void ForwardPass::Begin(const glm::vec3& clearColor)
{
	m_rt.Begin(clearColor);
	gl::Cmd::BindGraphicsPipeline(*m_pipeline);
}
//=============================================================================
void ForwardPass::End()
{
	m_rt.End();
}
//=============================================================================