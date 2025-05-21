#include "stdafx.h"
#include "OpenGL4Simple.h"
#include "Log.h"
//=============================================================================
#pragma region [ Local variables ]
//=============================================================================
namespace
{
	gl4::FrameBufferId currentFBO{ 0 };

	bool depthState{ false };
	bool blendingState{ false };
	gl4::PolygonMode polygonMode{ gl4::PolygonMode::Fill };
	gl4::DepthTestFunc depthTestFunc{ gl4::DepthTestFunc::Less };
	gl4::BlendFunc blendFunc{ gl4::BlendFunc::Zero };
}
//=============================================================================
void ClearOpenGLState()
{
	currentFBO = { 0 };
	depthState = { false };
	blendingState = { false };
	polygonMode = { gl4::PolygonMode::Fill };
	depthTestFunc = { gl4::DepthTestFunc::Less };
	blendFunc = { gl4::BlendFunc::Zero };
}
//=============================================================================
#pragma endregion
//=============================================================================
#pragma region [ Shader ]
//=============================================================================
inline std::string shaderTypeToString(GLenum shaderType)
{
	switch (shaderType)
	{
	case GL_VERTEX_SHADER:          return "GL_VERTEX_SHADER";
	case GL_FRAGMENT_SHADER:        return "GL_FRAGMENT_SHADER";
	case GL_GEOMETRY_SHADER:        return "GL_GEOMETRY_SHADER";
	case GL_TESS_CONTROL_SHADER:    return "GL_TESS_CONTROL_SHADER";
	case GL_TESS_EVALUATION_SHADER: return "GL_TESS_EVALUATION_SHADER";
	case GL_COMPUTE_SHADER:         return "GL_COMPUTE_SHADER";
	default:                        return "UNKNOWN_SHADER_TYPE";
	}
}
//=============================================================================
inline std::string printShaderSource(const char* text)
{
	int line = 1;
	std::string formatText = std::format("\n({:3d}): ", line);

	while (text && *text++)
	{
		if (*text == '\n')
		{
			formatText += std::format("\n({:3d}): ", ++line);
		}
		else if (*text == '\r')
		{
		}
		else
		{
			formatText += *text;
		}
	}
	return formatText;
}
//=============================================================================
inline void validateShader(GLuint id, GLenum type, const GLchar* shaderText)
{
	GLint success{};
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint infoLength{ 512 };
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLength);
		auto infoLog = std::string(infoLength + 1, '\0');
		glGetShaderInfoLog(id, infoLength, nullptr, infoLog.data());

		std::string logError = "OPENGL " + shaderTypeToString(type) + ": Shader compilation failed : " + infoLog;
		if (shaderText != nullptr)
			logError += ", Source: \n" + printShaderSource(shaderText);
		
		Error(logError);
	}
}
//=============================================================================
GLuint gl4::CreateShader(GLenum type, const std::string& source, std::string_view name)
{
	assert(!source.empty());

	const GLchar* shaderText = source.c_str();

	GLuint id = glCreateShader(type); assert(id);
	glShaderSource(id, 1, &shaderText, nullptr);
	glCompileShader(id);

	validateShader(id, type, shaderText);

	if (id && !name.empty())
	{
		glObjectLabel(GL_SHADER, id, static_cast<GLsizei>(name.length()), name.data());
	}

	return id;
}
//=============================================================================
GLuint gl4::CreateShaderSpirv(GLenum type, const ShaderSpirvInfo& spirvInfo, std::string_view name)
{
	GLuint id = glCreateShader(type); assert(id);

	glShaderBinary(1, &id, GL_SHADER_BINARY_FORMAT_SPIR_V, (const GLuint*)spirvInfo.code.data(), (GLsizei)spirvInfo.code.size_bytes());

	// Unzip specialization constants into two streams to feed to OpenGL
	auto indices = std::vector<uint32_t>(spirvInfo.specializationConstants.size());
	auto values = std::vector<uint32_t>(spirvInfo.specializationConstants.size());
	for (size_t i = 0; i < spirvInfo.specializationConstants.size(); i++)
	{
		indices[i] = spirvInfo.specializationConstants[i].index;
		values[i] = spirvInfo.specializationConstants[i].value;
	}

	glSpecializeShader(id, spirvInfo.entryPoint, (GLuint)spirvInfo.specializationConstants.size(), indices.data(), values.data());

	validateShader(id, type, nullptr);

	if (id && !name.empty())
	{
		glObjectLabel(GL_SHADER, id, static_cast<GLsizei>(name.length()), name.data());
	}

	return id;
}
//=============================================================================
std::string gl4::GetShaderSourceCode(GLuint id)
{
	if (glIsShader(id) != GL_TRUE)
	{
		assert(0);
		Fatal(std::to_string(id) + " not shader.");
		return "";
	}

	GLint length;
	glGetShaderiv(id, GL_SHADER_SOURCE_LENGTH, &length);
	std::vector<char> source(length);

	glGetShaderSource(id, length, nullptr, source.data());
	return std::string(source.data(), length);
}
//=============================================================================
#pragma endregion
//=============================================================================
#pragma region [ ShaderProgram ]
//=============================================================================
inline void checkProgramStatus(GLuint program)
{
	GLint success{};
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		GLint length{ 512 };
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
		auto infoLog = std::string(length + 1, '\0');
		glGetProgramInfoLog(program, length, nullptr, infoLog.data());
		Error("OPENGL: Shader Program(" + std::to_string(program) + ") linking failed: " + infoLog);
	}
}
//=============================================================================
gl4::ShaderProgramId gl4::CreateShaderProgram(const std::string& computeSrc)
{
	gl4::ShaderProgramId program;
	Create(program);
	GLuint shader = CreateShader(GL_COMPUTE_SHADER, computeSrc);
	glAttachShader(program, shader);
	glLinkProgram(program);
	glDeleteShader(shader);
	checkProgramStatus(program);
	return program;
}
//=============================================================================
gl4::ShaderProgramId gl4::CreateShaderProgram(const std::string& vertexSrc, const std::string& fragmentSrc)
{
	return CreateShaderProgram(vertexSrc, {}, fragmentSrc);
}
//=============================================================================
gl4::ShaderProgramId gl4::CreateShaderProgram(const std::string& vertexSrc, const std::string& geometrySrc, const std::string& fragmentSrc)
{
	gl4::ShaderProgramId program;
	Create(program);

	std::pair<GLenum, const std::string&> shaders[] = {
		{GL_VERTEX_SHADER,          vertexSrc},
		{GL_GEOMETRY_SHADER,        geometrySrc},
		{GL_FRAGMENT_SHADER,        fragmentSrc}
	};
	std::vector<GLuint> createdShaders;
	for (const auto& [type, source] : shaders)
	{
		if (!source.empty())
		{
			GLuint shader = CreateShader(type, source);
			glAttachShader(program, shader);
			createdShaders.push_back(shader);
		}
	}

	glLinkProgram(program);

	for (GLuint shader : createdShaders)
		glDeleteShader(shader);

	checkProgramStatus(program);
	return program;
}
//=============================================================================
void gl4::Bind(ShaderProgramId id)
{
	// TODO: возможно кеширование
	glUseProgram(id);
}
//=============================================================================
int gl4::GetUniformLocation(ShaderProgramId program, const std::string& name)
{
	return glGetUniformLocation(program, name.c_str());
}
//=============================================================================
GLuint gl4::GetUniformBlockIndex(ShaderProgramId program, const std::string& name)
{
	return glGetUniformBlockIndex(program, name.c_str());
}
//=============================================================================
#pragma region [ SetUniform ]
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, bool value)
{
	glProgramUniform1i(program, location, static_cast<int>(value));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, int value)
{
	glProgramUniform1i(program, location, value);
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, int v1, int v2)
{
	glProgramUniform2i(program, location, v1, v2);
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, int v1, int v2, int v3)
{
	glProgramUniform3i(program, location, v1, v2, v3);
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, int v1, int v2, int v3, int v4)
{
	glProgramUniform4i(program, location, v1, v2, v3, v4);
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, uint32_t value)
{
	glProgramUniform1ui(program, location, value);
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, uint32_t v1, uint32_t v2)
{
	glProgramUniform2ui(program, location, v1, v2);
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, uint32_t v1, uint32_t v2, uint32_t v3)
{
	glProgramUniform3ui(program, location, v1, v2, v3);
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4)
{
	glProgramUniform4ui(program, location, v1, v2, v3, v4);
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, float value)
{
	glProgramUniform1f(program, location, value);
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, float v1, float v2)
{
	glProgramUniform2f(program, location, v1, v2);
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, float v1, float v2, float v3)
{
	glProgramUniform3f(program, location, v1, v2, v3);
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, float v1, float v2, float v3, float v4)
{
	glProgramUniform4f(program, location, v1, v2, v3, v4);
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, const glm::vec2& value)
{
	glProgramUniform2fv(program, location, 1, glm::value_ptr(value));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, const glm::ivec2& value)
{
	glProgramUniform2iv(program, location, 1, glm::value_ptr(value));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, const glm::uvec2& value)
{
	glProgramUniform2uiv(program, location, 1, glm::value_ptr(value));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, const glm::vec3& value)
{
	glProgramUniform3fv(program, location, 1, glm::value_ptr(value));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, const glm::ivec3& value)
{
	glProgramUniform3iv(program, location, 1, glm::value_ptr(value));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, const glm::uvec3& value)
{
	glProgramUniform3uiv(program, location, 1, glm::value_ptr(value));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, const glm::vec4& value)
{
	glProgramUniform4fv(program, location, 1, glm::value_ptr(value));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, const glm::ivec4& value)
{
	glProgramUniform4iv(program, location, 1, glm::value_ptr(value));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, const glm::uvec4& value)
{
	glProgramUniform4uiv(program, location, 1, glm::value_ptr(value));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, const glm::mat2& mat, bool transpose)
{
	glProgramUniformMatrix2fv(program, location, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, const glm::mat3& mat, bool transpose)
{
	glProgramUniformMatrix3fv(program, location, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, const glm::mat4& mat, bool transpose)
{
	glProgramUniformMatrix4fv(program, location, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, const glm::mat2x3& mat, bool transpose)
{
	glProgramUniformMatrix2x3fv(program, location, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, const glm::mat3x2& mat, bool transpose)
{
	glProgramUniformMatrix3x2fv(program, location, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, const glm::mat2x4& mat, bool transpose)
{
	glProgramUniformMatrix2x4fv(program, location, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, const glm::mat4x2& mat, bool transpose)
{
	glProgramUniformMatrix4x2fv(program, location, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, const glm::mat3x4& mat, bool transpose)
{
	glProgramUniformMatrix3x4fv(program, location, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, int location, const glm::mat4x3& mat, bool transpose)
{
	glProgramUniformMatrix4x3fv(program, location, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, bool value)
{
	glProgramUniform1i(program, glGetUniformLocation(program, locName.c_str()), static_cast<int>(value));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, int value)
{
	glProgramUniform1i(program, glGetUniformLocation(program, locName.c_str()), value);
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, int v1, int v2)
{
	glProgramUniform2i(program, glGetUniformLocation(program, locName.c_str()), v1, v2);
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, int v1, int v2, int v3)
{
	glProgramUniform3i(program, glGetUniformLocation(program, locName.c_str()), v1, v2, v3);
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, int v1, int v2, int v3, int v4)
{
	glProgramUniform4i(program, glGetUniformLocation(program, locName.c_str()), v1, v2, v3, v4);
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, uint32_t value)
{
	glProgramUniform1ui(program, glGetUniformLocation(program, locName.c_str()), value);
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, uint32_t v1, uint32_t v2)
{
	glProgramUniform2ui(program, glGetUniformLocation(program, locName.c_str()), v1, v2);
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, uint32_t v1, uint32_t v2, uint32_t v3)
{
	glProgramUniform3ui(program, glGetUniformLocation(program, locName.c_str()), v1, v2, v3);
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4)
{
	glProgramUniform4ui(program, glGetUniformLocation(program, locName.c_str()), v1, v2, v3, v4);
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, float value)
{
	glProgramUniform1f(program, glGetUniformLocation(program, locName.c_str()), value);
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, float v1, float v2)
{
	glProgramUniform2f(program, glGetUniformLocation(program, locName.c_str()), v1, v2);
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, float v1, float v2, float v3)
{
	glProgramUniform3f(program, glGetUniformLocation(program, locName.c_str()), v1, v2, v3);
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, float v1, float v2, float v3, float v4)
{
	glProgramUniform4f(program, glGetUniformLocation(program, locName.c_str()), v1, v2, v3, v4);
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, const glm::vec2& value)
{
	glProgramUniform2fv(program, glGetUniformLocation(program, locName.c_str()), 1, glm::value_ptr(value));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, const glm::ivec2& value)
{
	glProgramUniform2iv(program, glGetUniformLocation(program, locName.c_str()), 1, glm::value_ptr(value));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, const glm::uvec2& value)
{
	glProgramUniform2uiv(program, glGetUniformLocation(program, locName.c_str()), 1, glm::value_ptr(value));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, const glm::vec3& value)
{
	glProgramUniform3fv(program, glGetUniformLocation(program, locName.c_str()), 1, glm::value_ptr(value));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, const glm::ivec3& value)
{
	glProgramUniform3iv(program, glGetUniformLocation(program, locName.c_str()), 1, glm::value_ptr(value));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, const glm::uvec3& value)
{
	glProgramUniform3uiv(program, glGetUniformLocation(program, locName.c_str()), 1, glm::value_ptr(value));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, const glm::vec4& value)
{
	glProgramUniform4fv(program, glGetUniformLocation(program, locName.c_str()), 1, glm::value_ptr(value));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, const glm::ivec4& value)
{
	glProgramUniform4iv(program, glGetUniformLocation(program, locName.c_str()), 1, glm::value_ptr(value));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, const glm::uvec4& value)
{
	glProgramUniform4uiv(program, glGetUniformLocation(program, locName.c_str()), 1, glm::value_ptr(value));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat2& mat, bool transpose)
{
	glProgramUniformMatrix2fv(program, glGetUniformLocation(program, locName.c_str()), 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat3& mat, bool transpose)
{
	glProgramUniformMatrix3fv(program, glGetUniformLocation(program, locName.c_str()), 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat4& mat, bool transpose)
{
	glProgramUniformMatrix4fv(program, glGetUniformLocation(program, locName.c_str()), 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat2x3& mat, bool transpose)
{
	glProgramUniformMatrix2x3fv(program, glGetUniformLocation(program, locName.c_str()), 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat3x2& mat, bool transpose)
{
	glProgramUniformMatrix3x2fv(program, glGetUniformLocation(program, locName.c_str()), 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat2x4& mat, bool transpose)
{
	glProgramUniformMatrix2x4fv(program, glGetUniformLocation(program, locName.c_str()), 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat4x2& mat, bool transpose)
{
	glProgramUniformMatrix4x2fv(program, glGetUniformLocation(program, locName.c_str()), 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat3x4& mat, bool transpose)
{
	glProgramUniformMatrix3x4fv(program, glGetUniformLocation(program, locName.c_str()), 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl4::SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat4x3& mat, bool transpose)
{
	glProgramUniformMatrix4x3fv(program, glGetUniformLocation(program, locName.c_str()), 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
#pragma endregion
//=============================================================================
#pragma endregion
//=============================================================================
#pragma region [ Buffer ]
//=============================================================================
gl4::BufferId gl4::CreateBuffer(GLenum usage, GLsizeiptr size, const void* data)
{
	gl4::BufferId buffer;
	gl4::Create(buffer);
	glNamedBufferData(buffer, size, data, usage);
	return buffer;
}
//=============================================================================
gl4::BufferId gl4::CreateBufferStorage(GLbitfield flags, GLsizeiptr size, const void* data)
{
	gl4::BufferId buffer;
	gl4::Create(buffer);
	glNamedBufferStorage(buffer, size, data, flags);
	return buffer;
}
//=============================================================================
gl4::BufferId gl4::CreateBufferStorage(GLbitfield flags, GLsizeiptr sizeElement, GLsizeiptr numElement, const void* data)
{
	return CreateBufferStorage(flags, sizeElement * numElement, data);
}
//=============================================================================
void gl4::SetSubData(BufferId id, GLintptr offset, GLsizeiptr size, const void* data)
{
	glNamedBufferSubData(id, offset, size, data);
}
//=============================================================================
void gl4::CopySubData(BufferId readBuffer, BufferId writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
	glCopyNamedBufferSubData(readBuffer, writeBuffer, readOffset, writeOffset, size);
}
//=============================================================================
void gl4::ClearData(BufferId id, GLenum internalFormat, GLenum format, GLenum type, const void* data)
{
	glClearNamedBufferData(id, internalFormat, format, type, data);
}
//=============================================================================
void gl4::ClearSubData(BufferId id, GLenum internalFormat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data)
{
	glClearNamedBufferSubData(id, internalFormat, offset, size, format, type, data);
}
//=============================================================================
void gl4::InvalidateData(BufferId id)
{
	glInvalidateBufferData(id);
}
//=============================================================================
void gl4::InvalidateSubData(BufferId id, GLintptr offset, GLsizeiptr length)
{
	glInvalidateBufferSubData(id, offset, length);
}
//=============================================================================
void* gl4::Map(BufferId id, GLenum access)
{
	return glMapNamedBuffer(id, access);
}
//=============================================================================
void* gl4::MapRange(BufferId id, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
	return glMapNamedBufferRange(id, offset, length, access);
}
//=============================================================================
bool gl4::UnMap(BufferId id)
{
	return glUnmapNamedBuffer(id) == GL_TRUE;
}
//=============================================================================
void gl4::FlushMappedRange(BufferId id, GLintptr offset, GLsizeiptr length)
{
	glFlushMappedNamedBufferRange(id, offset, length);
}
//=============================================================================
void* gl4::GetBufferPointer(BufferId id)
{
	void* ptr;
	glGetNamedBufferPointerv(id, GL_BUFFER_MAP_POINTER, &ptr);
	return ptr;
}
//=============================================================================
void gl4::GetSubData(BufferId id, GLintptr offset, GLsizeiptr size, void* data)
{
	glGetNamedBufferSubData(id, offset, size, data);
}
//=============================================================================
void gl4::BindBufferBase(BufferId id, GLenum target, GLuint index)
{
	glBindBufferBase(target, index, id);
}
//=============================================================================
void gl4::BindBufferRange(BufferId id, GLenum target, GLuint index, GLintptr offset, GLsizeiptr size)
{
	glBindBufferRange(target, index, id, offset, size);
}
//=============================================================================
#pragma endregion
//=============================================================================
#pragma region [ Vertex Array ]
//=============================================================================
void gl4::SetVertexAttrib(GLuint vao, GLuint attribIndex, GLint size, GLenum type, GLboolean normalized, GLuint relativeOffset, GLuint bindingIndex)
{
	// TODO: еще есть glVertexArrayBindingDivisor для инстанса

	glEnableVertexArrayAttrib(vao, attribIndex);
	glVertexArrayAttribBinding(vao, attribIndex, bindingIndex);

	if (type == GL_INT) // TODO: другие типы возможно тоже учесть, и есть еще glVertexArrayAttribLFormat
		glVertexArrayAttribIFormat(vao, attribIndex, size, type, relativeOffset);
	else
		glVertexArrayAttribFormat(vao, attribIndex, size, type, normalized, relativeOffset);
}
//=============================================================================
void gl4::SetVertexAttrib(GLuint vao, const VertexAttribute& attr)
{
	SetVertexAttrib(vao, attr.index, attr.size, attr.type, attr.normalized ? GL_TRUE : GL_FALSE, attr.relativeOffset, attr.bindingIndex);
}
//=============================================================================
void gl4::SetVertexAttrib(GLuint vao, const std::vector<VertexAttribute>& attributes)
{
	for (size_t i = 0; i < attributes.size(); i++)
	{
		SetVertexAttrib(vao, attributes[i]);
	}
}
//=============================================================================
gl4::VertexArrayId gl4::CreateVertexArray()
{
	gl4::VertexArrayId vao;
	gl4::Create(vao);
	return vao;
}
//=============================================================================
gl4::VertexArrayId gl4::CreateVertexArray(const std::vector<VertexAttribute>& attributes)
{
	gl4::VertexArrayId vao = CreateVertexArray();
	SetVertexAttrib(vao, attributes);
	return vao;
}
//=============================================================================
gl4::VertexArrayId gl4::CreateVertexArray(gl4::BufferId vbo, size_t vertexSize, const std::vector<VertexAttribute>& attributes)
{
	return CreateVertexArray(vbo, { 0 }, vertexSize, attributes);
}
//=============================================================================
gl4::VertexArrayId gl4::CreateVertexArray(gl4::BufferId vbo, gl4::BufferId ibo, size_t vertexSize, const std::vector<VertexAttribute>& attributes)
{
	gl4::VertexArrayId vao = CreateVertexArray(attributes);
	SetVertexBuffer(vao, vbo, 0, 0, vertexSize);
	SetIndexBuffer(vao, ibo);
	return vao;
}
//=============================================================================
void gl4::SetVertexBuffer(VertexArrayId id, BufferId vbo, GLuint bindingindex, GLintptr offset, GLsizei stride)
{
	// TODO: возможно кеширование
	if (IsValid(vbo)) glVertexArrayVertexBuffer(id, bindingindex, vbo, offset, stride);
}
//=============================================================================
void gl4::SetIndexBuffer(VertexArrayId id, BufferId ibo)
{
	if (IsValid(ibo)) glVertexArrayElementBuffer(id, ibo);
}
//=============================================================================
void gl4::Bind(VertexArrayId id)
{
	// TODO: возможно кеширование
	glBindVertexArray(id);
}
//=============================================================================
#pragma endregion
//=============================================================================
#pragma region [ Texture ]
//=============================================================================
inline GLenum getBaseFormat(GLenum internalFormat)
{
	switch (internalFormat)
	{
	case GL_R8:
	case GL_R8_SNORM:
	case GL_R16:
	case GL_R16_SNORM:
	case GL_R16F:
	case GL_R32F:
	case GL_R8I:
	case GL_R8UI:
	case GL_R16I:
	case GL_R16UI:
	case GL_R32I:
	case GL_R32UI:
		return GL_RED;

	case GL_RG8:
	case GL_RG8_SNORM:
	case GL_RG16:
	case GL_RG16_SNORM:
	case GL_RG16F:
	case GL_RG32F:
	case GL_RG8I:
	case GL_RG8UI:
	case GL_RG16I:
	case GL_RG16UI:
	case GL_RG32I:
	case GL_RG32UI:
		return GL_RG;

	case GL_R3_G3_B2:
	case GL_RGB4:
	case GL_RGB5:
	case GL_RGB8:
	case GL_RGB8_SNORM:
	case GL_RGB10:
	case GL_RGB12:
	case GL_RGB16_SNORM:
	case GL_RGBA2:
	case GL_RGBA4:
	case GL_SRGB8:
	case GL_RGB16F:
	case GL_RGB32F:
	case GL_R11F_G11F_B10F:
	case GL_RGB9_E5:
	case GL_RGB8I:
	case GL_RGB8UI:
	case GL_RGB16I:
	case GL_RGB16UI:
	case GL_RGB32I:
	case GL_RGB32UI:
		return GL_RGB;

	case GL_RGB5_A1:
	case GL_RGBA8:
	case GL_RGBA8_SNORM:
	case GL_RGB10_A2:
	case GL_RGB10_A2UI:
	case GL_RGBA12:
	case GL_RGBA16:
	case GL_SRGB8_ALPHA8:
	case GL_RGBA16F:
	case GL_RGBA32F:
	case GL_RGBA8I:
	case GL_RGBA8UI:
	case GL_RGBA16I:
	case GL_RGBA16UI:
	case GL_RGBA32I:
	case GL_RGBA32UI:
		return GL_RGBA;

	case GL_DEPTH_COMPONENT16:
	case GL_DEPTH_COMPONENT24:
	case GL_DEPTH_COMPONENT32F:
		return GL_DEPTH_COMPONENT;

	case GL_DEPTH24_STENCIL8:
	case GL_DEPTH32F_STENCIL8:
		return GL_DEPTH_STENCIL;

	case GL_STENCIL_INDEX8:
		return GL_STENCIL_INDEX;
	}

	return 0;
}
//=============================================================================
inline int getNumMipMapLevels2D(int width, int height)
{
	return static_cast<int>(floor(log2(std::max(width, height)))) + 1;
}
//=============================================================================
gl4::Texture2DId gl4::CreateTexture2D(GLenum internalFormat, GLsizei width, GLsizei height, void* data, const TextureParameter& param)
{
	const int numMipmaps = param.genMipMap ? getNumMipMapLevels2D(width, height) : 1;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	gl4::Texture2DId texture;
	gl4::Create(texture);
	glTextureParameteri(texture, GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, param.minFilter);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, param.magFilter);
	glTextureParameteri(texture, GL_TEXTURE_MAX_ANISOTROPY, param.maxAnisotropy);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_S, param.wrap);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_T, param.wrap);

	glTextureStorage2D(texture, numMipmaps, internalFormat, width, height);
	if (data)
	{
		SetSubImage(texture, 0, 0, 0, width, height, getBaseFormat(internalFormat), param.dataType, data);
		glGenerateTextureMipmap(texture);
	}

	return texture;
}
//=============================================================================
// Draw a checkerboard on a pre-allocated square RGB image.
inline uint8_t* genDefaultCheckerboardImage(int* width, int* height, int* nrChannels)
{
	const int w = 64;
	const int h = 64;

	uint8_t* imgData = (uint8_t*)malloc(w * h * 3); // stbi_load() uses malloc(), so this is safe
	for (int i = 0; i < w * h; i++)
	{
		const int row = i / w;
		const int col = i % w;
		imgData[i * 3 + 0] = imgData[i * 3 + 1] = imgData[i * 3 + 2] = 0xFF * ((row + col) % 2);
	}

	if (width) *width = w;
	if (height) *height = h;
	if (nrChannels) *nrChannels = 3;

	return imgData;
}
//=============================================================================
gl4::Texture2DId gl4::LoadTexture2D(const char* texturePath, bool flipVertical, const TextureParameter& param)
{
	stbi_set_flip_vertically_on_load(flipVertical);

	int width, height, nrChannels;
	stbi_uc* data = stbi_load(texturePath, &width, &height, &nrChannels, 0);
	if (!data)
	{
		Error((std::string("Texture: ") + texturePath + " not find").c_str());
		data = genDefaultCheckerboardImage(&width, &height, &nrChannels);
		if (!data)
		{
			Fatal("out of memory allocating image for fallback texture");
			return { 0 };
		}
	}

	GLenum internalFormat = GL_RGBA8;
	if (nrChannels == 1)      internalFormat = GL_R8;
	else if (nrChannels == 2) internalFormat = GL_RG8;
	else if (nrChannels == 3) internalFormat = GL_RGB8;

	gl4::Texture2DId texture = CreateTexture2D(internalFormat, width, height, data, param);

	stbi_image_free(data);
	return texture;
}
//=============================================================================
gl4::Texture2DId gl4::LoadTexture2DHDR(const char* texturePath, bool flipVertical, const TextureParameter& param)
{
	// TODO: возможно объединить с LoadTexture2D

	stbi_set_flip_vertically_on_load(flipVertical);

	int width, height, nrChannels;
	float* data = stbi_loadf(texturePath, &width, &height, &nrChannels, 0);
	if (!data)
	{
		Error((std::string("Texture: ") + texturePath + " not find").c_str());
		// TODO: создание дефолтной текстуры
		return { 0 };
	}
	gl4::Texture2DId texture = CreateTexture2D(GL_RGB32F, width, height, data, param);
	stbi_image_free(data);
	return texture;
}
//=============================================================================
gl4::TextureCubeId gl4::LoadCubeMap(const std::vector<std::string>& files, const std::string& directory)
{
	// TODO: возможность настроить через TextureParameter

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	gl4::TextureCubeId texture;
	gl4::Create(texture);

	stbi_set_flip_vertically_on_load(false);
	int width, height;

	for (uint32_t i = 0; i < files.size(); ++i)
	{
		auto filePath = directory + files[i];
		uint8_t* data = stbi_load(filePath.c_str(), &width, &height, nullptr, STBI_rgb_alpha);
		if (i == 0)
		{
			// Allocate the memory and set the format
			glTextureStorage2D(texture, 1, GL_RGBA8, width, height);
		}
		if (data)
		{
			// Upload the data
			glTextureSubImage3D(texture,
				0,
				0,
				0,
				static_cast<int>(i),
				width,
				height,
				1,
				GL_RGBA,
				GL_UNSIGNED_BYTE, data);
		}
		else
		{
			stbi_image_free(data);
			Error("Cubemap texture failed to load: " + files[i]);
			return { 0 };
		}
		stbi_image_free(data);
	}
	glTextureParameteri(texture, GL_TEXTURE_MAX_LEVEL, 0);
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return texture;
}
//=============================================================================
void gl4::SetSubImage(Texture1DId id, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels)
{
	glTextureSubImage1D(id, level, xoffset, width, format, type, pixels);
}
//=============================================================================
void gl4::SetSubImage(Texture2DId id, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
	glTextureSubImage2D(id, level, xoffset, yoffset, width, height, format, type, pixels);
}
//=============================================================================
void gl4::SetSubImage(Texture3DId id, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels)
{
	glTextureSubImage3D(id, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}
//=============================================================================
void gl4::SetCompressedSubImage(Texture1DId id, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data)
{
	glCompressedTextureSubImage1D(id, level, xoffset, width, format, imageSize, data);
}
//=============================================================================
void gl4::SetCompressedSubImage(Texture2DId id, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data)
{
	glCompressedTextureSubImage2D(id, level, xoffset, yoffset, width, height, format, imageSize, data);
}
//=============================================================================
void gl4::SetCompressedSubImage(Texture3DId id, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data)
{
	glCompressedTextureSubImage3D(id, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}
//=============================================================================
void gl4::CopySubImage(Texture1DId id, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
	glCopyTextureSubImage1D(id, level, xoffset, x, y, width);
}
//=============================================================================
void gl4::CopySubImage(Texture2DId id, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	glCopyTextureSubImage2D(id, level, xoffset, yoffset, x, y, width, height);
}
//=============================================================================
void gl4::CopySubImage(Texture3DId id, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	glCopyTextureSubImage3D(id, level, xoffset, yoffset, zoffset, x, y, width, height);
}
//=============================================================================
void gl4::Bind(GLuint unit, Texture1DId id)
{
	// кешировать
	glBindTextureUnit(unit, id);
}
//=============================================================================
void gl4::Bind(GLuint unit, Texture2DId id)
{
	// кешировать
	glBindTextureUnit(unit, id);
}
//=============================================================================
void gl4::Bind(GLuint unit, Texture3DId id)
{
	// кешировать
	glBindTextureUnit(unit, id);
}
//=============================================================================
void gl4::Bind(GLuint unit, TextureCubeId id)
{
	// кешировать
	glBindTextureUnit(unit, id);
}
//=============================================================================
#pragma endregion
//=============================================================================
#pragma region [ Sampler ]
//=============================================================================
gl4::SamplerId gl4::CreateSampler()
{
	SamplerId id;
	Create(id);
	return id;
}
//=============================================================================
void gl4::Bind(GLuint unit, SamplerId sampler)
{
	// TODO: кеширование
	glBindSampler(unit, sampler);
}
//=============================================================================
void gl4::Bind(GLuint unit, Texture2DId texture, SamplerId sampler)
{
	Bind(unit, texture);
	Bind(unit, sampler);
}
//=============================================================================
#pragma endregion
//=============================================================================
#pragma region [ RenderBuffer ]
//=============================================================================
gl4::RenderBufferId gl4::CreateRenderBuffer(GLenum internalFormat, GLsizei width, GLsizei height)
{
	RenderBufferId id;
	Create(id);
	glNamedRenderbufferStorage(id, internalFormat, width, height);
	return id;
}
//=============================================================================
gl4::RenderBufferId gl4::CreateRenderBuffer(GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height)
{
	RenderBufferId id;
	Create(id);
	glNamedRenderbufferStorageMultisample(id, samples, internalFormat, width, height);
	return id;
}
//=============================================================================
#pragma endregion
//=============================================================================
#pragma region [ FrameBuffer ]
//=============================================================================
GLuint gl4::CreateColorBuffer2D(int width, int height, GLenum formatColor)
{
	TextureParameter param = defaultTextureParameter2D;
	param.wrap = GL_CLAMP_TO_EDGE;
	return gl4::CreateTexture2D(formatColor, width, height, nullptr, param);
}
//=============================================================================
GLuint gl4::CreateDepthBuffer2D(int width, int height, GLenum formatDepth)
{
	TextureParameter param = {};
	param.minFilter = GL_NEAREST;
	param.magFilter = GL_NEAREST;
	param.wrap = GL_CLAMP_TO_BORDER;
	param.genMipMap = false;
	param.dataType = GL_FLOAT;

	GLuint texture = gl4::CreateTexture2D(formatDepth, width, height, nullptr, param);
	constexpr GLfloat border[]{ 1.0f, 1.0f, 1.0f, 1.0f };
	glTextureParameterfv(texture, GL_TEXTURE_BORDER_COLOR, border);

	return texture;
}
//=============================================================================
gl4::FrameBufferId gl4::CreateFrameBuffer2D(GLuint colorBuffer, GLuint depthBuffer)
{
	gl4::FrameBufferId framebuffer;
	gl4::Create(framebuffer);

	if (colorBuffer > 0)
		glNamedFramebufferTexture(framebuffer, GL_COLOR_ATTACHMENT0, colorBuffer, 0);

	if (depthBuffer > 0)
		glNamedFramebufferTexture(framebuffer, GL_DEPTH_ATTACHMENT, depthBuffer, 0);

	const GLenum status = glCheckNamedFramebufferStatus(framebuffer, GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		// TODO: error
	}

	return framebuffer;
}
//=============================================================================
gl4::FrameBufferId gl4::CreateFrameBuffer2D(Texture2DId colorBuffer, Texture2DId depthBuffer)
{
	gl4::FrameBufferId framebuffer;
	gl4::Create(framebuffer);

	if (IsValid(colorBuffer))
	{
		glNamedFramebufferTexture(framebuffer, GL_COLOR_ATTACHMENT0, colorBuffer, 0);
	}

	if (IsValid(depthBuffer))
	{
		glNamedFramebufferTexture(framebuffer, GL_DEPTH_ATTACHMENT, depthBuffer, 0);
	}

	const GLenum status = glCheckNamedFramebufferStatus(framebuffer, GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		// TODO: error
	}

	return framebuffer;
}
//=============================================================================
gl4::FrameBufferId gl4::CreateFrameBuffer2D(Texture2DId colorBuffer, RenderBufferId depthBuffer)
{
	gl4::FrameBufferId framebuffer;
	gl4::Create(framebuffer);

	if (IsValid(colorBuffer))
	{
		glNamedFramebufferTexture(framebuffer, GL_COLOR_ATTACHMENT0, colorBuffer, 0);
	}

	if (IsValid(depthBuffer))
	{
		glNamedFramebufferRenderbuffer(framebuffer, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	}

	const GLenum status = glCheckNamedFramebufferStatus(framebuffer, GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		// TODO: error
	}

	return framebuffer;
}
//=============================================================================
void gl4::SetDrawBuffer(FrameBufferId fbo, GLenum buffer)
{
	glNamedFramebufferDrawBuffer(fbo, buffer);
}
//=============================================================================
void gl4::SetDrawBuffers(FrameBufferId fbo, GLsizei size, const GLenum* buffers)
{
	glNamedFramebufferDrawBuffers(fbo, size, buffers);
}
//=============================================================================
void gl4::Invalidate(FrameBufferId fbo, GLsizei numAttachments, const GLenum* attachments)
{
	glInvalidateNamedFramebufferData(fbo, numAttachments, attachments);
}
//=============================================================================
void gl4::InvalidateSubData(FrameBufferId fbo, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
	glInvalidateNamedFramebufferSubData(fbo, numAttachments, attachments, x, y, width, height);
}
//=============================================================================
void gl4::SetFrameBuffer(FrameBufferId fbo)
{
	if (currentFBO != fbo)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		currentFBO = fbo;
	}
}
//=============================================================================
void gl4::SetFrameBuffer(FrameBufferId fbo, int width, int height, GLbitfield clearMask)
{
	SetFrameBuffer(fbo);
	glViewport(0, 0, width, height);
	glClear(clearMask);
}
//=============================================================================
#pragma endregion
//=============================================================================
#pragma region [ State ]
//=============================================================================
void gl4::SwitchDepthTestState(bool state)
{
	if (state != depthState)
	{
		if (state) glEnable(GL_DEPTH_TEST);
		else glDisable(GL_DEPTH_TEST);
		depthState = state;
	}
}
//=============================================================================
void gl4::SwitchBlendingState(bool state)
{
	if (state != blendingState)
	{
		if (state) glEnable(GL_BLEND);
		else glDisable(GL_BLEND);
		blendingState = state;
	}
}
//=============================================================================
void gl4::SwitchPolygonMode(PolygonMode mode)
{
	constexpr GLenum glPolygonModes[] = { GL_POINT, GL_LINE, GL_FILL };
	if (mode != polygonMode)
	{
		unsigned int indexMode = static_cast<unsigned int>(mode);
		glPolygonMode(GL_FRONT_AND_BACK, glPolygonModes[indexMode]);
		polygonMode = mode;
	}
}
//=============================================================================
void gl4::SwitchDepthTestFunc(DepthTestFunc mode)
{
	constexpr GLenum glDepthFuncs[] = { GL_LESS, GL_NEVER, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS };
	if (mode != depthTestFunc)
	{
		unsigned int indexFunc = static_cast<unsigned int>(mode);
		glDepthFunc(glDepthFuncs[indexFunc]);
		depthTestFunc = mode;
	}
}
//=============================================================================
void gl4::SwitchBlendingFunc(BlendFunc mode)
{
	GLenum glBlendFuncs[] = {	GL_ZERO,           GL_ONE,
								GL_SRC_COLOR,      GL_ONE_MINUS_SRC_COLOR,
								GL_DST_COLOR,      GL_ONE_MINUS_DST_COLOR,
								GL_SRC_ALPHA,      GL_ONE_MINUS_SRC_ALPHA,
								GL_DST_ALPHA,      GL_ONE_MINUS_DST_ALPHA,
								GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR,
								GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA };
	if (mode != blendFunc)
	{
		unsigned int indexFunc = static_cast<unsigned int>(mode);
		glBlendFunc(GL_SRC_ALPHA, glBlendFuncs[indexFunc]);
		blendFunc = mode;
	}
}
//=============================================================================
#pragma endregion