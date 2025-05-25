#pragma once

#include "OpenGL4Core.h"

namespace gl4
{
	class Shader;

	struct InputAssemblyState final
	{
		PrimitiveTopology topology = PrimitiveTopology::TRIANGLE_LIST;
		bool primitiveRestartEnable = false;

		bool operator==(const InputAssemblyState&) const noexcept = default;
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

		bool operator==(const TessellationState&) const noexcept = default;
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

		bool operator==(const RasterizationState&) const noexcept = default;
	};

	struct MultisampleState final
	{
		bool sampleShadingEnable{ false };   // glEnable(GL_SAMPLE_SHADING)
		float minSampleShading{ 1 };         // glMinSampleShading
		uint32_t sampleMask{ 0xFFFFFFFF };   // glSampleMaski
		bool alphaToCoverageEnable{ false }; // glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE)
		bool alphaToOneEnable{ false };      // glEnable(GL_SAMPLE_ALPHA_TO_ONE)

		bool operator==(const MultisampleState&) const noexcept = default;
	};

	struct DepthState final
	{
		bool depthTestEnable{ false };               // gl{Enable, Disable}(GL_DEPTH_TEST)
		bool depthWriteEnable{ false };              // glDepthMask(depthWriteEnable)
		CompareOp depthCompareOp{ CompareOp::Less }; // glDepthFunc

		bool operator==(const DepthState&) const noexcept = default;
	};

	struct StencilOpState final
	{
		StencilOp passOp{ StencilOp::Keep };      // glStencilOp (dppass)
		StencilOp failOp{ StencilOp::Keep };      // glStencilOp (sfail)
		StencilOp depthFailOp{ StencilOp::Keep }; // glStencilOp (dpfail)
		CompareOp compareOp{ CompareOp::Always }; // glStencilFunc (func)
		uint32_t compareMask{ 0 };                // glStencilFunc (mask)
		uint32_t writeMask{ 0 };                  // glStencilMask
		uint32_t reference{ 0 };                  // glStencilFunc (ref)

		bool operator==(const StencilOpState&) const noexcept = default;
	};

	struct StencilState final
	{
		bool stencilTestEnable{ false };
		StencilOpState front{};
		StencilOpState back{};

		bool operator==(const StencilState&) const noexcept = default;
	};

	struct ColorBlendAttachmentState final // glBlendFuncSeparatei + glBlendEquationSeparatei
	{
		bool blendEnable{ false };                                           // if false, blend factor = one?
		BlendFactor srcColorBlendFactor{ BlendFactor::One };              // srcRGB
		BlendFactor dstColorBlendFactor{ BlendFactor::Zero };             // dstRGB
		BlendOp colorBlendOp{ BlendOp::Add };                  // modeRGB
		BlendFactor srcAlphaBlendFactor{ BlendFactor::One };              // srcAlpha
		BlendFactor dstAlphaBlendFactor{ BlendFactor::Zero };             // dstAlpha
		BlendOp alphaBlendOp{ BlendOp::Add };                  // modeAlpha
		ColorComponentFlags colorWriteMask{ ColorComponentFlag::RGBA_BITS }; // glColorMaski

		bool operator==(const ColorBlendAttachmentState&) const noexcept = default;
	};

	struct ColorBlendState final
	{
		bool logicOpEnable{ false };          // gl{Enable, Disable}(GL_COLOR_LOGIC_OP)
		LogicOp logicOp{ LogicOp::Copy };  // glLogicOp(logicOp)
		std::vector<ColorBlendAttachmentState> attachments{};             // glBlendFuncSeparatei + glBlendEquationSeparatei
		float blendConstants[4] = { 0, 0, 0, 0 }; // glBlendColor

		bool operator==(const ColorBlendState&) const noexcept = default;
	};

	/// @brief Parameters for the constructor of GraphicsPipeline
	struct GraphicsPipelineInfo final
	{
		/// @brief An optional name for viewing in a graphics debugger
		std::string_view name;

		/// @brief Non-null pointer to a vertex shader
		const Shader* vertexShader = nullptr;

		/// @brief Optional pointer to a fragment shader
		const Shader* fragmentShader = nullptr;

		/// @brief Optional pointer to a tessellation control shader
		const Shader* tessellationControlShader = nullptr;

		/// @brief Optional pointer to a tessellation evaluation shader
		const Shader* tessellationEvaluationShader = nullptr;

		InputAssemblyState inputAssemblyState = {};
		VertexInputState vertexInputState = {};
		TessellationState tessellationState = {};
		RasterizationState rasterizationState = {};
		MultisampleState multisampleState = {};
		DepthState depthState = {};
		StencilState stencilState = {};
		ColorBlendState colorBlendState = {};
	};

	/// @brief Parameters for the constructor of ComputePipeline
	struct ComputePipelineInfo final
	{
		/// @brief An optional name for viewing in a graphics debugger
		std::string_view name;

		/// @brief Non-null pointer to a compute shader
		const Shader* shader;
	};

	/// @brief An object that encapsulates the state needed to issue draws
	struct GraphicsPipeline final
	{
		/// @throws PipelineCompilationException
		explicit GraphicsPipeline(const GraphicsPipelineInfo& info);
		~GraphicsPipeline();
		GraphicsPipeline(GraphicsPipeline&& old) noexcept;
		GraphicsPipeline& operator=(GraphicsPipeline&& old) noexcept;
		GraphicsPipeline(const GraphicsPipeline&) = delete;
		GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

		bool operator==(const GraphicsPipeline&) const = default;

		[[nodiscard]] uint64_t Handle() const { return m_id; }

	private:
		uint64_t m_id;
	};

	/// @brief An object that encapsulates the state needed to issue dispatches
	struct ComputePipeline final
	{
		/// @throws PipelineCompilationException
		explicit ComputePipeline(const ComputePipelineInfo& info);
		~ComputePipeline();
		ComputePipeline(ComputePipeline&& old) noexcept;
		ComputePipeline& operator=(ComputePipeline&& old) noexcept;
		ComputePipeline(const ComputePipeline&) = delete;
		ComputePipeline& operator=(const ComputePipeline&) = delete;

		bool operator==(const ComputePipeline&) const = default;

		[[nodiscard]] Extent3D WorkgroupSize() const;

		[[nodiscard]] uint64_t Handle() const { return m_id; }

	private:
		uint64_t m_id;
	};

} // namespace gl4