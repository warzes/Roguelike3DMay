#include "stdafx.h"
#include "DepthPass.h"
#include "Uniforms.h"
#include "DirectionalLight.h"
//=============================================================================
constexpr uint16_t DepthMapSize = 4096;
//=============================================================================
bool DepthPass::Init()
{
	m_depthDataUBO.Init();

	m_rt.Init(DepthMapSize, DepthMapSize, RTAttachment{ gl::Format::D32_FLOAT, "DepthPass", gl::AttachmentLoadOp::Clear });

	auto vertexShader = gl::Shader(gl::ShaderType::VertexShader, io::ReadShaderCode("CuteGameData/shaders/depth.vert"), "DepthPassVS");

	m_pipeline = gl::GraphicsPipeline({
		 .name              = "DepthPassPipeline",
		.vertexShader       = &vertexShader,
		.inputAssemblyState = {.topology = gl::PrimitiveTopology::TriangleList },
		.vertexInputState   = { MeshVertexInputBindingDesc },
		.depthState         = {.depthTestEnable = true },
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
	if (gDirectionalLight.IsNeedsUpdate())
	{
		m_depthDataUBO->vp = gDirectionalLight.GetMatrix();
		m_depthDataUBO.Update();
	}

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