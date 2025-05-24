#pragma once

namespace gl4
{
	enum class PipelineStage : uint8_t
	{
		VertexShader,
		TessellationControlShader,
		TessellationEvaluationShader,
		FragmentShader,
		ComputeShader,
	};

	struct SpecializationConstant final
	{
		uint32_t index{ 0 };
		uint32_t value{ 0 };
	};

	struct ShaderSpirvInfo final
	{
		const char* entryPoint{ "main" };
		std::span<const uint32_t> code;
		std::span<const SpecializationConstant> specializationConstants;
	};

	/// @brief A shader object to be used in one or more GraphicsPipeline or ComputePipeline objects
	class Shader final
	{
	public:
		/// @brief Constructs a shader from GLSL
		/// @param stage A pipeline stage
		/// @param source A GLSL source string
		/// @param name An optional debug identifier
		/// @throws ShaderCompilationException if the shader is malformed
		explicit Shader(PipelineStage stage, std::string_view source, std::string_view name = "");
		/// @brief Constructs a shader from SPIR-V
		explicit Shader(PipelineStage stage, const ShaderSpirvInfo& spirvInfo, std::string_view name = "");

		Shader(const Shader&) = delete;
		Shader(Shader&& old) noexcept;
		Shader& operator=(const Shader&) = delete;
		Shader& operator=(Shader&& old) noexcept;
		~Shader();

		[[nodiscard]] bool IsValid() const { return m_id > 0; }

		[[nodiscard]] GLuint Handle() const { return m_id; }
		[[nodiscard]] operator GLuint() const { return m_id; }

	private:
		GLuint m_id{0};
	};

} // namespace gl4