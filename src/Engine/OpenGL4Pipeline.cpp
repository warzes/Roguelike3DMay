#include "stdafx.h"
#include "OpenGL4Pipeline.h"
#include "OpenGL4PipelineManager.h"
#include "Log.h"
//=============================================================================
gl4::GraphicsPipeline::GraphicsPipeline(const GraphicsPipelineInfo& info)
	: m_id(detail::CompileGraphicsPipelineInternal(info))

{
	Debug("Created Graphics Program with handle " + std::to_string(m_id));
}
//=============================================================================
gl4::GraphicsPipeline::~GraphicsPipeline()
{
	if (m_id != 0)
	{
		Debug("Destroyed Graphics Program with handle " + std::to_string(m_id));
		detail::DestroyGraphicsPipelineInternal(m_id);
	}
}
//=============================================================================
gl4::GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& old) noexcept : m_id(std::exchange(old.m_id, 0)) {}
//=============================================================================
gl4::GraphicsPipeline& gl4::GraphicsPipeline::operator=(GraphicsPipeline&& old) noexcept
{
	if (this == &old)
	{
		return *this;
	}

	m_id = std::exchange(old.m_id, 0);
	return *this;
}
//=============================================================================
gl4::ComputePipeline::ComputePipeline(const ComputePipelineInfo& info)
	: m_id(detail::CompileComputePipelineInternal(info))
{
	Debug("Created Compute Program with handle " + std::to_string(m_id));
}
//=============================================================================
gl4::ComputePipeline::~ComputePipeline()
{
	if (m_id != 0)
	{
		Debug("Destroyed Compute program with handle " + std::to_string(m_id));
		detail::DestroyComputePipelineInternal(m_id);
	}
}
//=============================================================================
gl4::ComputePipeline::ComputePipeline(ComputePipeline&& old) noexcept
	: m_id(std::exchange(old.m_id, 0))
{
}
//=============================================================================
gl4::ComputePipeline& gl4::ComputePipeline::operator=(ComputePipeline&& old) noexcept
{
	if (this == &old)
	{
		return *this;
	}

	m_id = std::exchange(old.m_id, 0);
	return *this;
}
//=============================================================================
Extent3D gl4::ComputePipeline::WorkgroupSize() const
{
	return detail::GetComputePipelineInternal(m_id)->workgroupSize;
}
//=============================================================================