#pragma once

#include "OpenGL4Core.h"
#include "BasicTypes.h"

namespace gl
{
	class Shader;

	struct InputAssemblyState final
	{
		PrimitiveTopology topology{ PrimitiveTopology::TriangleList };
		bool              primitiveRestartEnable{ false };
	};

	struct VertexInputBindingDescription final
	{
		uint32_t location; // glEnableVertexArrayAttrib + glVertexArrayAttribFormat
		uint32_t binding;  // glVertexArrayAttribBinding
		Format   format;   // glVertexArrayAttribFormat
		uint32_t offset;   // glVertexArrayAttribFormat
	};

	struct VertexInputState final
	{
		std::span<const VertexInputBindingDescription> vertexBindingDescriptions = {};
	};

	struct TessellationState final
	{
		uint32_t patchControlPoints; // glPatchParameteri(GL_PATCH_VERTICES, ...)
	};

	struct RasterizationState final
	{
		bool        depthClampEnable{ false };
		PolygonMode polygonMode{ PolygonMode::Fill };
		CullMode    cullMode{ CullMode::Back };
		FrontFace   frontFace{ FrontFace::CounterClockwise };
		bool        depthBiasEnable{ false };
		float       depthBiasConstantFactor{ 0 };
		float       depthBiasSlopeFactor{ 0 };
		float       lineWidth{ 1 }; // glLineWidth
		float       pointSize{ 1 }; // glPointSize
	};

	struct MultisamplingState final
	{
		bool     sampleShadingEnable{ false };   // glEnable(GL_SAMPLE_SHADING)
		float    minSampleShading{ 1 };          // glMinSampleShading
		uint32_t sampleMask{ 0xFFFFFFFF };       // glSampleMaski
		bool     alphaToCoverageEnable{ false }; // glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE)
		bool     alphaToOneEnable{ false };      // glEnable(GL_SAMPLE_ALPHA_TO_ONE)
	};

	struct DepthState final
	{
		bool      depthTestEnable{ false };          // gl{Enable, Disable}(GL_DEPTH_TEST)
		bool      depthWriteEnable{ false };         // glDepthMask(depthWriteEnable)
		CompareOp depthCompareOp{ CompareOp::Less }; // glDepthFunc
	};

	struct StencilOpState final
	{
		bool operator==(const StencilOpState&) const noexcept = default;

		StencilOp passOp{ StencilOp::Keep };      // glStencilOp (dppass)
		StencilOp failOp{ StencilOp::Keep };      // glStencilOp (sfail)
		StencilOp depthFailOp{ StencilOp::Keep }; // glStencilOp (dpfail)
		CompareOp compareOp{ CompareOp::Always }; // glStencilFunc (func)
		uint32_t  compareMask{ 0 };               // glStencilFunc (mask)
		uint32_t  writeMask{ 0 };                 // glStencilMask
		uint32_t  reference{ 0 };                 // glStencilFunc (ref)
	};

	struct StencilState final
	{
		bool           stencilTestEnable{ false };
		StencilOpState front{};
		StencilOpState back{};
	};

	struct ColorBlendAttachmentState final // glBlendFuncSeparatei + glBlendEquationSeparatei
	{
		bool operator==(const ColorBlendAttachmentState&) const noexcept = default;

		bool                blendEnable{ false };                           // if false, blend factor = one?
		BlendFactor         srcColorBlendFactor{ BlendFactor::One };        // srcRGB
		BlendFactor         dstColorBlendFactor{ BlendFactor::Zero };       // dstRGB
		BlendOp             colorBlendOp{ BlendOp::Add };                   // modeRGB
		BlendFactor         srcAlphaBlendFactor{ BlendFactor::One };        // srcAlpha
		BlendFactor         dstAlphaBlendFactor{ BlendFactor::Zero };       // dstAlpha
		BlendOp             alphaBlendOp{ BlendOp::Add };                   // modeAlpha
		ColorComponentFlags colorWriteMask{ ColorComponentFlag::RGBABits }; // glColorMaski
	};

	struct ColorBlendState final
	{
		bool                                   logicOpEnable{ false };             // gl{Enable, Disable}(GL_COLOR_LOGIC_OP)
		LogicOp                                logicOp{ LogicOp::Copy };           // glLogicOp(logicOp)
		std::vector<ColorBlendAttachmentState> attachments{};                      // glBlendFuncSeparatei + glBlendEquationSeparatei
		float                                  blendConstants[4] = { 0, 0, 0, 0 }; // glBlendColor
	};

	struct GraphicsPipelineInfo final
	{
		std::string_view name{};

		const Shader* vertexShader{ nullptr };
		const Shader* fragmentShader{ nullptr };
		const Shader* tessellationControlShader{ nullptr };
		const Shader* tessellationEvaluationShader{ nullptr };

		InputAssemblyState inputAssemblyState = {};
		VertexInputState   vertexInputState = {};
		TessellationState  tessellationState = {};
		RasterizationState rasterizationState = {};
		MultisamplingState multisamplingState = {};
		DepthState         depthState = {};
		StencilState       stencilState = {};
		ColorBlendState    colorBlendState = {};
	};

	struct ComputePipelineInfo final
	{
		std::string_view name{};
		const Shader*    shader{ nullptr };
	};

	struct GraphicsPipeline final
	{
		explicit GraphicsPipeline(const GraphicsPipelineInfo& info);
		~GraphicsPipeline();
		GraphicsPipeline(GraphicsPipeline&& old) noexcept;
		GraphicsPipeline& operator=(GraphicsPipeline&& old) noexcept;
		GraphicsPipeline(const GraphicsPipeline&) = delete;
		GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

		[[nodiscard]] uint64_t Handle() const { return m_id; }

	private:
		uint64_t m_id{ 0 };
	};

	struct ComputePipeline final
	{
		explicit ComputePipeline(const ComputePipelineInfo& info);
		~ComputePipeline();
		ComputePipeline(ComputePipeline&& old) noexcept;
		ComputePipeline& operator=(ComputePipeline&& old) noexcept;
		ComputePipeline(const ComputePipeline&) = delete;
		ComputePipeline& operator=(const ComputePipeline&) = delete;


		[[nodiscard]] Extent3D WorkgroupSize() const;
		[[nodiscard]] uint64_t Handle() const { return m_id; }

	private:
		uint64_t m_id{ 0 };
	};

} // namespace gl

namespace gl::detail
{
	struct VertexInputStateOwning final
	{
		std::vector<VertexInputBindingDescription> vertexBindingDescriptions;
	};

	struct GraphicsPipelineInfoOwning final
	{
		std::string                                   name;
		InputAssemblyState                            inputAssemblyState;
		VertexInputStateOwning                        vertexInputState;
		TessellationState                             tessellationState;
		RasterizationState                            rasterizationState;
		MultisamplingState                            multisamplingState;
		DepthState                                    depthState;
		StencilState                                  stencilState;
		ColorBlendState                               colorBlendState;
		std::vector<std::pair<std::string, uint32_t>> uniformBlocks;
		std::vector<std::pair<std::string, uint32_t>> storageBlocks;
		std::vector<std::pair<std::string, uint32_t>> samplersAndImages;
	};

	struct ComputePipelineInfoOwning final
	{
		std::string                                   name;
		Extent3D                                      workgroupSize;
		std::vector<std::pair<std::string, uint32_t>> uniformBlocks;
		std::vector<std::pair<std::string, uint32_t>> storageBlocks;
		std::vector<std::pair<std::string, uint32_t>> samplersAndImages;
	};

	std::shared_ptr<const GraphicsPipelineInfoOwning> GetGraphicsPipelineInternal(uint64_t pipeline);
	std::shared_ptr<const ComputePipelineInfoOwning> GetComputePipelineInternal(uint64_t pipeline);
} // namespace gl::detail