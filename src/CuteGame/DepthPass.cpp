#include "stdafx.h"
#include "DepthPass.h"
#include "Uniforms.h"
//=============================================================================
namespace
{
	const char* shaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec3 vertexPosition;

layout(binding = 0, std140) uniform DepthBlock {
	mat4 vp;
};

layout(binding = 1, std140) uniform ModelMatricesBlock {
	mat4 modelMatrix;
};

void main()
{
	gl_Position = vp * modelMatrix * vec4(vertexPosition, 1.0);
}
)";
}
//=============================================================================
bool DepthPass::Init()
{
	m_depthDataUBO.Init();

	m_rt.Init(2048, 2048, std::span<RTAttachment>{}, RTAttachment{ gl::Format::D32_FLOAT, "DepthPass", gl::AttachmentLoadOp::Clear });

	auto vertexShader = gl::Shader(gl::ShaderType::VertexShader, shaderCodeVertex, "DepthPassVS");

	m_pipeline = gl::GraphicsPipeline({
		 .name = "DepthPassPipeline",
		.vertexShader = &vertexShader,
		.inputAssemblyState = {.topology = gl::PrimitiveTopology::TriangleList },
		.vertexInputState = { MeshVertexInputBindingDesc },
		.depthState = {.depthTestEnable = true },
		});

	return true;
}
//=============================================================================
void DepthPass::Close()
{
	m_depthDataUBO.Close();
	m_rt.Close();
	m_pipeline = std::nullopt;
}
//=============================================================================
void DepthPass::Begin()
{
	glm::mat4 viewMatrix = glm::lookAt(glm::vec3(1.0f, 5.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 projectionMatrix = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 1.0f, 1000.0f);
	m_depthDataUBO->vp = projectionMatrix * viewMatrix;
	
	m_depthDataUBO.Update();

	m_rt.Begin({});
	gl::Cmd::BindGraphicsPipeline(*m_pipeline);
	m_depthDataUBO.Bind(0);
}
//=============================================================================
void DepthPass::End()
{
	m_rt.End();
}
//=============================================================================
const gl::Texture* DepthPass::GetTexture() const
{
	return m_rt.GetDepth();
}
//=============================================================================