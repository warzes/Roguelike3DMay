#pragma once

namespace gl
{
	enum class ShaderType : uint8_t
	{
		VertexShader,                 // GL_VERTEX_SHADER
		GeometryShader,               // GL_GEOMETRY_SHADER
		TessellationControlShader,    // GL_TESS_CONTROL_SHADER
		TessellationEvaluationShader, // GL_TESS_EVALUATION_SHADER
		FragmentShader,               // GL_FRAGMENT_SHADER
		ComputeShader,                // GL_COMPUTE_SHADER
	};

	struct SpecializationConstant final
	{
		uint32_t index{ 0 };
		uint32_t value{ 0 };
	};

	struct ShaderSpirvInfo final
	{
		std::string_view                        entryPoint{ "main" };
		std::span<const uint32_t>               code;
		std::span<const SpecializationConstant> specializationConstants;
	};

	// A shader object to be used in one or more GraphicsPipeline or ComputePipeline objects
	class Shader final
	{
	public:
		explicit Shader(ShaderType stage, std::string_view source, std::string_view name = "");
		explicit Shader(ShaderType stage, const ShaderSpirvInfo& spirvInfo, std::string_view name = "");

		Shader(const Shader&) = delete;
		Shader(Shader&& old) noexcept;
		Shader& operator=(const Shader&) = delete;
		Shader& operator=(Shader&& old) noexcept;
		~Shader();

		[[nodiscard]] bool IsValid() const noexcept { return m_id > 0; }

		[[nodiscard]] GLuint Handle() const noexcept { return m_id; }
		[[nodiscard]] operator GLuint() const noexcept { return m_id; }

		[[nodiscard]] std::string GetShaderSourceCode() const;

	private:
		GLuint m_id{0};
	};

} // namespace gl