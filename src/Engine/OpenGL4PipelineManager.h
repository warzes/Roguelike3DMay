#pragma once

#include "OpenGL4Pipeline.h"

namespace gl4::detail
{
	// owning versions of pipeline info structs so we don't lose references
	struct VertexInputStateOwning final
	{
		std::vector<VertexInputBindingDescription> vertexBindingDescriptions;
	};

	struct ColorBlendStateOwning final
	{
		bool logicOpEnable;
		LogicOp logicOp;
		std::vector<ColorBlendAttachmentState> attachments;
		float blendConstants[4];
	};

	struct GraphicsPipelineInfoOwning final
	{
		std::string name;
		InputAssemblyState inputAssemblyState;
		VertexInputStateOwning vertexInputState;
		TessellationState tessellationState;
		RasterizationState rasterizationState;
		MultisampleState multisampleState;
		DepthState depthState;
		StencilState stencilState;
		ColorBlendStateOwning colorBlendState;
		std::vector<std::pair<std::string, uint32_t>> uniformBlocks;
		std::vector<std::pair<std::string, uint32_t>> storageBlocks;
		std::vector<std::pair<std::string, uint32_t>> samplersAndImages;
	};

	struct ComputePipelineInfoOwning final
	{
		std::string name;
		Extent3D workgroupSize;
		std::vector<std::pair<std::string, uint32_t>> uniformBlocks;
		std::vector<std::pair<std::string, uint32_t>> storageBlocks;
		std::vector<std::pair<std::string, uint32_t>> samplersAndImages;
	};

	uint64_t CompileGraphicsPipelineInternal(const GraphicsPipelineInfo& info);
	std::shared_ptr<const GraphicsPipelineInfoOwning> GetGraphicsPipelineInternal(uint64_t pipeline);
	void DestroyGraphicsPipelineInternal(uint64_t pipeline);

	uint64_t CompileComputePipelineInternal(const ComputePipelineInfo& info);
	std::shared_ptr<const ComputePipelineInfoOwning> GetComputePipelineInternal(uint64_t pipeline);
	void DestroyComputePipelineInternal(uint64_t pipeline);
} // namespace gl4::detail