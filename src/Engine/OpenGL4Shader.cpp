#include "stdafx.h"
#include "OpenGL4Shader.h"
#include "OpenGL4DeviceProperties.h"
#include "Log.h"
//=============================================================================
[[nodiscard]] inline std::string shaderStageToString(gl::ShaderType stage)
{
	switch (stage)
	{
	case gl::ShaderType::VertexShader:                 return "GL_VERTEX_SHADER";
	case gl::ShaderType::FragmentShader:               return "GL_FRAGMENT_SHADER";
	case gl::ShaderType::TessellationControlShader:    return "GL_TESS_CONTROL_SHADER";
	case gl::ShaderType::TessellationEvaluationShader: return "GL_TESS_EVALUATION_SHADER";
	case gl::ShaderType::ComputeShader:                return "GL_COMPUTE_SHADER";
	default: assert(0);                                return "UNKNOWN_SHADER_TYPE";
	}
}
//=============================================================================
[[nodiscard]] inline GLenum enumToGL(gl::ShaderType stage)
{
	switch (stage)
	{
	case gl::ShaderType::VertexShader:                 return GL_VERTEX_SHADER;
	case gl::ShaderType::TessellationControlShader:    return GL_TESS_CONTROL_SHADER;
	case gl::ShaderType::TessellationEvaluationShader: return GL_TESS_EVALUATION_SHADER;
	case gl::ShaderType::FragmentShader:               return GL_FRAGMENT_SHADER;
	case gl::ShaderType::ComputeShader:                return GL_COMPUTE_SHADER;
	default: assert(0);                                return 0;
	}
}
//=============================================================================
[[nodiscard]] inline std::string printShaderSource(const char* text)
{
	if (!text) return "";

	std::ostringstream oss;
	int line = 1;
	oss << "\n(" << std::setw(3) << std::setfill(' ') << line << "): ";

	while (*text)
	{
		if (*text == '\n')
		{
			oss << '\n';
			line++;
			oss << "(" << std::setw(3) << std::setfill(' ') << line << "): ";
		}
		else if (*text != '\r')
		{
			oss << *text;
		}
		text++;
	}
	return oss.str();
}
//=============================================================================
inline void validateShader(GLuint& id, gl::ShaderType stage, const char* shaderText)
{
	GLint success{};
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint infoLength{ 0 };
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLength);
		std::string infoLog;
		if (infoLength > 1)
		{
			infoLog.resize(static_cast<size_t>(infoLength - 1)); // исключаем \0
			glGetShaderInfoLog(id, infoLength, nullptr, infoLog.data());
		}
		else
		{
			infoLog = "<no info log>";
		}

		std::string logError = "OPENGL " + shaderStageToString(stage) + ": Shader compilation failed: " + infoLog;
		if (shaderText != nullptr) logError += ", Source: \n" + printShaderSource(shaderText);
		Error(logError);
		glDeleteShader(id);
		id = 0;
	}
}
//=============================================================================
[[nodiscard]] inline GLuint compileShaderGLSL(gl::ShaderType stage, std::string_view sourceGLSL)
{
	const GLchar* strings = sourceGLSL.data();
	GLuint id = glCreateShader(enumToGL(stage));
	if (id == 0)
	{
		Error("Failed to create OpenGL shader object for stage: " + std::string(shaderStageToString(stage)));
		return 0;
	}

	glShaderSource(id, 1, &strings, nullptr);
	glCompileShader(id);
	validateShader(id, stage, strings);
	return id;
}
//=============================================================================
[[nodiscard]] inline GLuint compileShaderSpirv(gl::ShaderType stage, const gl::ShaderSpirvInfo& spirvInfo)
{
	if (!gl::CurrentDeviceProperties.features.spirv)
	{
		Error("SPIR-V is not supported");
		return 0;
	}

	if (spirvInfo.code.empty())
	{
		Error("SPIR-V code is empty");
		return 0;
	}

	GLuint id = glCreateShader(enumToGL(stage));
	if (id == 0)
	{
		Error("Failed to create OpenGL shader object for stage: " + std::string(shaderStageToString(stage)));
		return 0;
	}

	glShaderBinary(1, &id, GL_SHADER_BINARY_FORMAT_SPIR_V, static_cast<const void*>(spirvInfo.code.data()), static_cast<GLsizei>(spirvInfo.code.size_bytes()));
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		Error("glShaderBinary failed with error: " + std::to_string(error));
		glDeleteShader(id);
		return 0;
	}

	// Unzip specialization constants into two streams to feed to OpenGL
	if (!spirvInfo.specializationConstants.empty())
	{
		auto indices = std::vector<uint32_t>(spirvInfo.specializationConstants.size());
		auto values = std::vector<uint32_t>(spirvInfo.specializationConstants.size());
		for (size_t i = 0; i < spirvInfo.specializationConstants.size(); i++)
		{
			indices[i] = spirvInfo.specializationConstants[i].index;
			values[i] = spirvInfo.specializationConstants[i].value;
		}
		glSpecializeShader(id, spirvInfo.entryPoint.data(), static_cast<GLuint>(indices.size()), indices.data(), values.data());
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			Error("glSpecializeShader failed with error: " + std::to_string(error));
			glDeleteShader(id);
			return 0;
		}
	}
	
	validateShader(id, stage, nullptr);
	return id;
}
//=============================================================================
gl::Shader::Shader(ShaderType stage, std::string_view source, std::string_view name)
{
	m_id = compileShaderGLSL(stage, source);
	if (!m_id) return;

	if (!name.empty())
		glObjectLabel(GL_SHADER, m_id, static_cast<GLsizei>(name.length()), name.data());

	Debug("Created Shader with handle " + std::to_string(m_id));
}
//=============================================================================
gl::Shader::Shader(ShaderType stage, const ShaderSpirvInfo& spirvInfo, std::string_view name)
{
	m_id = compileShaderSpirv(stage, spirvInfo);
	if (!m_id) return;

	if (!name.empty())
		glObjectLabel(GL_SHADER, m_id, static_cast<GLsizei>(name.length()), name.data());

	Debug("Created Shader with handle " + std::to_string(m_id));
}
//=============================================================================
gl::Shader::Shader(Shader&& old) noexcept : m_id(std::exchange(old.m_id, 0)) {}
//=============================================================================
gl::Shader& gl::Shader::operator=(Shader&& old) noexcept
{
	if (this != &old)
	{
		this->~Shader();
		m_id = std::exchange(old.m_id, 0);
	}
	return *this;
}
//=============================================================================
gl::Shader::~Shader()
{
	Debug("Destroyed Shader with handle " + std::to_string(m_id));
	glDeleteShader(m_id);
	m_id = 0;
}
//=============================================================================
std::string gl::Shader::GetShaderSourceCode() const
{
	assert(IsValid() && "Trying to get source from invalid shader");
	if (!IsValid()) return {};

	GLint length{ 0 };
	glGetShaderiv(m_id, GL_SHADER_SOURCE_LENGTH, &length);
	if (length <= 1) return {}; // 1 = только \0

	GLint actualLength = 0;
	std::vector<char> source(static_cast<size_t>(length), '\0');
	glGetShaderSource(m_id, length, &actualLength, source.data());
	return std::string(source.data(), static_cast<size_t>(actualLength > 0 ? actualLength - 1 : 0));
}
//=============================================================================