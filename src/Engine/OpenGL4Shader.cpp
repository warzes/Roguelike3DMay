#include "stdafx.h"
#include "OpenGL4Shader.h"
#include "OpenGL4ApiToEnum.h"
#include "Log.h"
//=============================================================================
namespace
{
	inline std::string printShaderSource(const char* text)
	{
		int line = 1;
		std::string formatText = std::format("\n({:3d}): ", line);

		while (text && *text++)
		{
			if (*text == '\n') { formatText += std::format("\n({:3d}): ", ++line); }
			else if (*text == '\r') {}
			else { formatText += *text; }
		}
		return formatText;
	}

	inline void validateShader(GLuint& id, gl4::PipelineStage stage, const GLchar* shaderText)
	{
		GLint success{};
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);
		if (success == GL_FALSE)
		{
			GLint infoLength{ 512 };
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLength);
			auto infoLog = std::string(static_cast<size_t>(infoLength + 1), '\0');
			glGetShaderInfoLog(id, infoLength, nullptr, infoLog.data());

			std::string logError = "OPENGL " + gl4::detail::ShaderStageToString(stage) + ": Shader compilation failed : " + infoLog;
			if (shaderText != nullptr) logError += ", Source: \n" + printShaderSource(shaderText);
			Error(logError);
			glDeleteShader(id);
			id = 0;
		}
	}

	inline GLuint compileShaderGLSL(gl4::PipelineStage stage, std::string_view sourceGLSL)
	{
		const GLchar* strings = sourceGLSL.data();
		GLuint id = glCreateShader(gl4::detail::EnumToGL(stage));
		glShaderSource(id, 1, &strings, nullptr);
		glCompileShader(id);
		validateShader(id, stage, strings);
		return id;
	}

	inline GLuint compileShaderSpirv(gl4::PipelineStage stage, const gl4::ShaderSpirvInfo& spirvInfo)
	{
		GLuint id = glCreateShader(gl4::detail::EnumToGL(stage));
		glShaderBinary(1, &id, GL_SHADER_BINARY_FORMAT_SPIR_V, (const GLuint*)spirvInfo.code.data(), static_cast<GLsizei>(spirvInfo.code.size_bytes()));

		// Unzip specialization constants into two streams to feed to OpenGL
		auto indices = std::vector<uint32_t>(spirvInfo.specializationConstants.size());
		auto values = std::vector<uint32_t>(spirvInfo.specializationConstants.size());
		for (size_t i = 0; i < spirvInfo.specializationConstants.size(); i++)
		{
			indices[i] = spirvInfo.specializationConstants[i].index;
			values[i] = spirvInfo.specializationConstants[i].value;
		}
		glSpecializeShader(id, spirvInfo.entryPoint, static_cast<GLuint>(spirvInfo.specializationConstants.size()), indices.data(), values.data());
		validateShader(id, stage, nullptr);
		return id;
	}
}
//=============================================================================
gl4::Shader::Shader(PipelineStage stage, std::string_view source, std::string_view name)
{
	m_id = compileShaderGLSL(stage, source);
	if (m_id && !name.empty())
		glObjectLabel(GL_SHADER, m_id, static_cast<GLsizei>(name.length()), name.data());

	Debug("Created Shader with handle " + std::to_string(m_id));
}
//=============================================================================
gl4::Shader::Shader(PipelineStage stage, const ShaderSpirvInfo& spirvInfo, std::string_view name)
{
	m_id = compileShaderSpirv(stage, spirvInfo);
	if (m_id && !name.empty())
		glObjectLabel(GL_SHADER, m_id, static_cast<GLsizei>(name.length()), name.data());

	Debug("Created Shader with handle " + std::to_string(m_id));
}
//=============================================================================
gl4::Shader::Shader(Shader&& old) noexcept : m_id(std::exchange(old.m_id, 0)) {}
//=============================================================================
gl4::Shader& gl4::Shader::operator=(Shader&& old) noexcept
{
	if (&old == this)
		return *this;
	this->~Shader();
	return *new (this) Shader(std::move(old));
}
//=============================================================================
gl4::Shader::~Shader()
{
	Debug("Destroyed Shader with handle " + std::to_string(m_id));
	glDeleteShader(m_id);
}
//=============================================================================
std::string gl4::Shader::GetShaderSourceCode() const
{
	GLint length;
	glGetShaderiv(m_id, GL_SHADER_SOURCE_LENGTH, &length);
	std::vector<char> source(static_cast<size_t>(length));

	glGetShaderSource(m_id, length, nullptr, source.data());
	return std::string(source.data(), static_cast<size_t>(length));
}
//=============================================================================