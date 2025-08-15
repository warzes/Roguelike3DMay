#include "stdafx.h"
#include "OpenGL4Simple.h"
#include "Engine/Log.h"
#include "Engine/OpenGL4DeviceProperties.h"
#include "Engine/Hash.h"
//=============================================================================
#pragma region [ Hash ]
//=============================================================================
inline size_t vertexInputStateHash(const gl::VertexInputState& k)
{
	size_t hashVal{};

	for (const auto& desc : k.vertexBindingDescriptions)
	{
		auto cctup = std::make_tuple(desc.location, desc.binding, desc.format, desc.offset);
		auto chashVal = detail::hashing::hash<decltype(cctup)>{}(cctup);
		detail::hashing::hash_combine(hashVal, chashVal);
	}

	return hashVal;
}
//=============================================================================
#pragma endregion
//=============================================================================
#pragma region [ Local variables ]
//=============================================================================
//struct GraphicsPipelineCache final
//{
//	std::string_view   debugName;
//
//	gl::InputAssemblyState inputAssemblyState;
//	gl::TessellationState  tessellationState;
//	gl::RasterizationState rasterizationState;
//	gl::MultisampleState   multisampleState;
//	gl::DepthState         depthState;
//	gl::StencilState       stencilState;
//	gl::ColorBlendState    colorBlendState;
//
//	bool valid{ false };
//
//	GraphicsPipelineCache& operator=(const GraphicsPipelineCache&) = default;
//	GraphicsPipelineCache& operator=(const gl::GraphicsPipelineId& p)
//	{
//		valid = true;
//
//		debugName = p.debugName;
//		inputAssemblyState = p.inputAssemblyState;
//		tessellationState = p.tessellationState;
//		rasterizationState = p.rasterizationState;
//		multisampleState = p.multisampleState;
//		depthState = p.depthState;
//		stencilState = p.stencilState;
//		colorBlendState = p.colorBlendState;
//
//		return *this;
//	}
//
//	bool operator==(const gl::GraphicsPipelineId& p) const noexcept
//	{
//		return (
//			valid == p.valid &&
//			debugName == p.debugName &&
//			inputAssemblyState == p.inputAssemblyState &&
//			tessellationState == p.tessellationState &&
//			rasterizationState == p.rasterizationState &&
//			multisampleState == p.multisampleState &&
//			depthState == p.depthState &&
//			stencilState == p.stencilState &&
//			colorBlendState == p.colorBlendState
//			);
//	}
//};
//=============================================================================
namespace
{
	// Used for scope error checking
	bool isComputeActive = false;
	bool isRendering = false;

	// Used for error checking for indexed draws
	bool isIndexBufferBound = false;

	// True during a render or compute scope that has a name.
	bool isScopedDebugGroupPushed = false;

	// True when a pipeline with a name is bound during a render or compute scope.
	bool isPipelineDebugGroupPushed = false;

	// True during SwapchainRendering scopes that disable sRGB.
	// This is needed since regular Rendering scopes always have framebuffer sRGB enabled
	// (the user uses framebuffer attachments to decide if they want the linear->sRGB conversion).
	bool srgbWasDisabled = false;
		
	gl::ShaderProgramId lastProgram; // шейдер отделен от pipeline так как есть еще вычислительный шейдер и долно сбрасывать

	// Potentially used for state deduplication.
	gl::VertexArrayId currentVao{ 0 };
	gl::FrameBufferId currentFBO{ 0 };

	// Currently unused (and probably shouldn't be used)
	const gl::RenderInfo* lastRenderInfo{};

	// These can be set at the start of rendering, so they need to be tracked separately from the other pipeline state.
	std::array<gl::ColorComponentFlags, MAX_COLOR_ATTACHMENTS> lastColorMask = {};
	bool lastDepthMask = true;
	uint32_t lastStencilMask[2] = { static_cast<uint32_t>(-1), static_cast<uint32_t>(-1) };
	bool initViewport = true;
	gl::Viewport lastViewport = {};
	Rect2D lastScissor = {};
	bool scissorEnabled = false;

	// These persist until another Pipeline is bound.
	// They are not used for state deduplication, as they are arguments for GL draw calls.
	gl::PrimitiveTopology currentTopology{};
	gl::IndexType currentIndexType{};

	std::unordered_map<size_t, gl::VertexArrayId> vertexArrayCache;
	std::unordered_map<gl::SamplerState, gl::SamplerId> samplerCache;
}
//=============================================================================
void ClearOpenGLState()
{
	isComputeActive = false;
	isRendering = false;
	isIndexBufferBound = false;
	isScopedDebugGroupPushed = false;
	if (isPipelineDebugGroupPushed)
	{
		isPipelineDebugGroupPushed = false;
		glPopDebugGroup();
	}
	srgbWasDisabled = false;

	lastProgram = {};
	currentVao = {};
	currentFBO = {};
	lastRenderInfo = nullptr;

	lastColorMask = {};
	lastDepthMask = true;
	lastStencilMask[0] = static_cast<uint32_t>(-1);
	lastStencilMask[1] = static_cast<uint32_t>(-1);

	initViewport = true;
	lastViewport = {};
	lastScissor = {};
	scissorEnabled = false;


	currentTopology = {};
	currentIndexType = {};
}
//=============================================================================
void ClearResourceCache()
{
	for (auto& [_, vao] : vertexArrayCache) { gl::Destroy(vao); }
	vertexArrayCache.clear();
	for (auto& [_, sampler] : samplerCache) { gl::Destroy(sampler); }
	samplerCache.clear();
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
		auto infoLog = std::string((size_t)infoLength + 1, '\0');
		glGetShaderInfoLog(id, infoLength, nullptr, infoLog.data());

		std::string logError = "OPENGL " + shaderTypeToString(type) + ": Shader compilation failed : " + infoLog;
		if (shaderText != nullptr)
			logError += ", Source: \n" + printShaderSource(shaderText);
		
		Error(logError);
	}
}
//=============================================================================
GLuint gl::CreateShader(GLenum type, const std::string& source, std::string_view name)
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
GLuint gl::CreateShaderSpirv(GLenum type, const ShaderSpirvInfo& spirvInfo, std::string_view name)
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

	glSpecializeShader(id, spirvInfo.entryPoint.data(), (GLuint)spirvInfo.specializationConstants.size(), indices.data(), values.data());

	validateShader(id, type, nullptr);

	if (id && !name.empty())
	{
		glObjectLabel(GL_SHADER, id, static_cast<GLsizei>(name.length()), name.data());
	}

	return id;
}
//=============================================================================
std::string gl::GetShaderSourceCode(GLuint id)
{
	if (glIsShader(id) != GL_TRUE)
	{
		assert(0);
		Fatal(std::to_string(id) + " not shader.");
		return "";
	}

	GLint length;
	glGetShaderiv(id, GL_SHADER_SOURCE_LENGTH, &length);
	std::vector<char> source((size_t)length);

	glGetShaderSource(id, length, nullptr, source.data());
	return std::string(source.data(), (size_t)length);
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
		auto infoLog = std::string((size_t)length + 1, '\0');
		glGetProgramInfoLog(program, length, nullptr, infoLog.data());
		Error("OPENGL: Shader Program(" + std::to_string(program) + ") linking failed: " + infoLog);
	}
}
//=============================================================================
gl::ShaderProgramId gl::CreateShaderProgram(const std::string& computeSrc)
{
	gl::ShaderProgramId program{ glCreateProgram() };
	GLuint shader = CreateShader(GL_COMPUTE_SHADER, computeSrc);
	glAttachShader(program, shader);
	glLinkProgram(program);
	glDeleteShader(shader);
	checkProgramStatus(program);
	return program;
}
//=============================================================================
gl::ShaderProgramId gl::CreateShaderProgram(const std::string& vertexSrc, const std::string& fragmentSrc)
{
	return CreateShaderProgram(vertexSrc, {}, fragmentSrc);
}
//=============================================================================
gl::ShaderProgramId gl::CreateShaderProgram(const std::string& vertexSrc, const std::string& geometrySrc, const std::string& fragmentSrc)
{
	gl::ShaderProgramId program{ glCreateProgram() };

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
void gl::Bind(ShaderProgramId id)
{
	// TODO: возможно кеширование
	glUseProgram(id);
}
//=============================================================================
int gl::GetUniformLocation(ShaderProgramId program, const std::string& name)
{
	return glGetUniformLocation(program, name.c_str());
}
//=============================================================================
GLuint gl::GetUniformBlockIndex(ShaderProgramId program, const std::string& name)
{
	return glGetUniformBlockIndex(program, name.c_str());
}
//=============================================================================
#pragma region [ SetUniform ]
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, bool value)
{
	glProgramUniform1i(program, location, static_cast<int>(value));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, int value)
{
	glProgramUniform1i(program, location, value);
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, int v1, int v2)
{
	glProgramUniform2i(program, location, v1, v2);
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, int v1, int v2, int v3)
{
	glProgramUniform3i(program, location, v1, v2, v3);
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, int v1, int v2, int v3, int v4)
{
	glProgramUniform4i(program, location, v1, v2, v3, v4);
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, uint32_t value)
{
	glProgramUniform1ui(program, location, value);
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, uint32_t v1, uint32_t v2)
{
	glProgramUniform2ui(program, location, v1, v2);
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, uint32_t v1, uint32_t v2, uint32_t v3)
{
	glProgramUniform3ui(program, location, v1, v2, v3);
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4)
{
	glProgramUniform4ui(program, location, v1, v2, v3, v4);
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, float value)
{
	glProgramUniform1f(program, location, value);
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, float v1, float v2)
{
	glProgramUniform2f(program, location, v1, v2);
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, float v1, float v2, float v3)
{
	glProgramUniform3f(program, location, v1, v2, v3);
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, float v1, float v2, float v3, float v4)
{
	glProgramUniform4f(program, location, v1, v2, v3, v4);
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, const glm::vec2& value)
{
	glProgramUniform2fv(program, location, 1, glm::value_ptr(value));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, const glm::ivec2& value)
{
	glProgramUniform2iv(program, location, 1, glm::value_ptr(value));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, const glm::uvec2& value)
{
	glProgramUniform2uiv(program, location, 1, glm::value_ptr(value));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, const glm::vec3& value)
{
	glProgramUniform3fv(program, location, 1, glm::value_ptr(value));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, const glm::ivec3& value)
{
	glProgramUniform3iv(program, location, 1, glm::value_ptr(value));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, const glm::uvec3& value)
{
	glProgramUniform3uiv(program, location, 1, glm::value_ptr(value));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, const glm::vec4& value)
{
	glProgramUniform4fv(program, location, 1, glm::value_ptr(value));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, const glm::ivec4& value)
{
	glProgramUniform4iv(program, location, 1, glm::value_ptr(value));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, const glm::uvec4& value)
{
	glProgramUniform4uiv(program, location, 1, glm::value_ptr(value));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, const glm::mat2& mat, bool transpose)
{
	glProgramUniformMatrix2fv(program, location, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, const glm::mat3& mat, bool transpose)
{
	glProgramUniformMatrix3fv(program, location, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, const glm::mat4& mat, bool transpose)
{
	glProgramUniformMatrix4fv(program, location, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, const glm::mat2x3& mat, bool transpose)
{
	glProgramUniformMatrix2x3fv(program, location, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, const glm::mat3x2& mat, bool transpose)
{
	glProgramUniformMatrix3x2fv(program, location, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, const glm::mat2x4& mat, bool transpose)
{
	glProgramUniformMatrix2x4fv(program, location, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, const glm::mat4x2& mat, bool transpose)
{
	glProgramUniformMatrix4x2fv(program, location, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, const glm::mat3x4& mat, bool transpose)
{
	glProgramUniformMatrix3x4fv(program, location, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, int location, const glm::mat4x3& mat, bool transpose)
{
	glProgramUniformMatrix4x3fv(program, location, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, bool value)
{
	glProgramUniform1i(program, glGetUniformLocation(program, locName.c_str()), static_cast<int>(value));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, int value)
{
	glProgramUniform1i(program, glGetUniformLocation(program, locName.c_str()), value);
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, int v1, int v2)
{
	glProgramUniform2i(program, glGetUniformLocation(program, locName.c_str()), v1, v2);
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, int v1, int v2, int v3)
{
	glProgramUniform3i(program, glGetUniformLocation(program, locName.c_str()), v1, v2, v3);
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, int v1, int v2, int v3, int v4)
{
	glProgramUniform4i(program, glGetUniformLocation(program, locName.c_str()), v1, v2, v3, v4);
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, uint32_t value)
{
	glProgramUniform1ui(program, glGetUniformLocation(program, locName.c_str()), value);
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, uint32_t v1, uint32_t v2)
{
	glProgramUniform2ui(program, glGetUniformLocation(program, locName.c_str()), v1, v2);
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, uint32_t v1, uint32_t v2, uint32_t v3)
{
	glProgramUniform3ui(program, glGetUniformLocation(program, locName.c_str()), v1, v2, v3);
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4)
{
	glProgramUniform4ui(program, glGetUniformLocation(program, locName.c_str()), v1, v2, v3, v4);
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, float value)
{
	glProgramUniform1f(program, glGetUniformLocation(program, locName.c_str()), value);
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, float v1, float v2)
{
	glProgramUniform2f(program, glGetUniformLocation(program, locName.c_str()), v1, v2);
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, float v1, float v2, float v3)
{
	glProgramUniform3f(program, glGetUniformLocation(program, locName.c_str()), v1, v2, v3);
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, float v1, float v2, float v3, float v4)
{
	glProgramUniform4f(program, glGetUniformLocation(program, locName.c_str()), v1, v2, v3, v4);
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, const glm::vec2& value)
{
	glProgramUniform2fv(program, glGetUniformLocation(program, locName.c_str()), 1, glm::value_ptr(value));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, const glm::ivec2& value)
{
	glProgramUniform2iv(program, glGetUniformLocation(program, locName.c_str()), 1, glm::value_ptr(value));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, const glm::uvec2& value)
{
	glProgramUniform2uiv(program, glGetUniformLocation(program, locName.c_str()), 1, glm::value_ptr(value));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, const glm::vec3& value)
{
	glProgramUniform3fv(program, glGetUniformLocation(program, locName.c_str()), 1, glm::value_ptr(value));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, const glm::ivec3& value)
{
	glProgramUniform3iv(program, glGetUniformLocation(program, locName.c_str()), 1, glm::value_ptr(value));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, const glm::uvec3& value)
{
	glProgramUniform3uiv(program, glGetUniformLocation(program, locName.c_str()), 1, glm::value_ptr(value));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, const glm::vec4& value)
{
	glProgramUniform4fv(program, glGetUniformLocation(program, locName.c_str()), 1, glm::value_ptr(value));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, const glm::ivec4& value)
{
	glProgramUniform4iv(program, glGetUniformLocation(program, locName.c_str()), 1, glm::value_ptr(value));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, const glm::uvec4& value)
{
	glProgramUniform4uiv(program, glGetUniformLocation(program, locName.c_str()), 1, glm::value_ptr(value));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat2& mat, bool transpose)
{
	glProgramUniformMatrix2fv(program, glGetUniformLocation(program, locName.c_str()), 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat3& mat, bool transpose)
{
	glProgramUniformMatrix3fv(program, glGetUniformLocation(program, locName.c_str()), 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat4& mat, bool transpose)
{
	glProgramUniformMatrix4fv(program, glGetUniformLocation(program, locName.c_str()), 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat2x3& mat, bool transpose)
{
	glProgramUniformMatrix2x3fv(program, glGetUniformLocation(program, locName.c_str()), 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat3x2& mat, bool transpose)
{
	glProgramUniformMatrix3x2fv(program, glGetUniformLocation(program, locName.c_str()), 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat2x4& mat, bool transpose)
{
	glProgramUniformMatrix2x4fv(program, glGetUniformLocation(program, locName.c_str()), 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat4x2& mat, bool transpose)
{
	glProgramUniformMatrix4x2fv(program, glGetUniformLocation(program, locName.c_str()), 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat3x4& mat, bool transpose)
{
	glProgramUniformMatrix3x4fv(program, glGetUniformLocation(program, locName.c_str()), 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat));
}
//=============================================================================
void gl::SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat4x3& mat, bool transpose)
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
gl::BufferId gl::CreateBuffer(GLenum usage, GLsizeiptr size, const void* data)
{
	gl::BufferId buffer;
	glCreateBuffers(1, &buffer.id);
	glNamedBufferData(buffer, size, data, usage);
	return buffer;
}
//=============================================================================
gl::BufferId gl::CreateBufferStorage(GLbitfield flags, GLsizeiptr size, const void* data)
{
	gl::BufferId buffer;
	glCreateBuffers(1, &buffer.id);
	glNamedBufferStorage(buffer, size, data, flags);
	return buffer;
}
//=============================================================================
gl::BufferId gl::CreateBufferStorage(GLbitfield flags, GLsizeiptr sizeElement, GLsizeiptr numElement, const void* data)
{
	return CreateBufferStorage(flags, sizeElement * numElement, data);
}
//=============================================================================
void gl::SetSubData(BufferId id, GLintptr offset, GLsizeiptr size, const void* data)
{
	glNamedBufferSubData(id, offset, size, data);
}
//=============================================================================
void gl::CopySubData(BufferId readBuffer, BufferId writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
	glCopyNamedBufferSubData(readBuffer, writeBuffer, readOffset, writeOffset, size);
}
//=============================================================================
void gl::ClearData(BufferId id, GLenum internalFormat, GLenum format, GLenum type, const void* data)
{
	glClearNamedBufferData(id, internalFormat, format, type, data);
}
//=============================================================================
void gl::ClearSubData(BufferId id, GLenum internalFormat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data)
{
	glClearNamedBufferSubData(id, internalFormat, offset, size, format, type, data);
}
//=============================================================================
void gl::InvalidateData(BufferId id)
{
	glInvalidateBufferData(id);
}
//=============================================================================
void gl::InvalidateSubData(BufferId id, GLintptr offset, GLsizeiptr length)
{
	glInvalidateBufferSubData(id, offset, length);
}
//=============================================================================
void* gl::Map(BufferId id, GLenum access)
{
	return glMapNamedBuffer(id, access);
}
//=============================================================================
void* gl::MapRange(BufferId id, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
	return glMapNamedBufferRange(id, offset, length, access);
}
//=============================================================================
bool gl::UnMap(BufferId id)
{
	return glUnmapNamedBuffer(id) == GL_TRUE;
}
//=============================================================================
void gl::FlushMappedRange(BufferId id, GLintptr offset, GLsizeiptr length)
{
	glFlushMappedNamedBufferRange(id, offset, length);
}
//=============================================================================
void* gl::GetBufferPointer(BufferId id)
{
	void* ptr;
	glGetNamedBufferPointerv(id, GL_BUFFER_MAP_POINTER, &ptr);
	return ptr;
}
//=============================================================================
void gl::GetSubData(BufferId id, GLintptr offset, GLsizeiptr size, void* data)
{
	glGetNamedBufferSubData(id, offset, size, data);
}
//=============================================================================
void gl::BindBufferBase(BufferId id, GLenum target, GLuint index)
{
	glBindBufferBase(target, index, id);
}
//=============================================================================
void gl::BindBufferRange(BufferId id, GLenum target, GLuint index, GLintptr offset, GLsizeiptr size)
{
	glBindBufferRange(target, index, id, offset, size);
}
//=============================================================================
#pragma endregion
//=============================================================================
#pragma region [ BufferStorage ]
//=============================================================================
constexpr size_t roundUp(size_t numberToRoundUp, size_t multipleOf)
{
	assert(multipleOf);
	return ((numberToRoundUp + multipleOf - 1) / multipleOf) * multipleOf;
}
//=============================================================================
//gl::BufferStorageId gl::CreateStorageBuffer(size_t size, BufferStorageFlags storageFlags, std::string_view name)
//{
//	return CreateStorageBuffer(nullptr, size, storageFlags, name);
//}
////=============================================================================
//gl::BufferStorageId gl::CreateStorageBuffer(TriviallyCopyableByteSpan data, BufferStorageFlags storageFlags, std::string_view name)
//{
//	return CreateStorageBuffer(data.data(), data.size_bytes(), storageFlags, name);
//}
////=============================================================================
//gl::BufferStorageId gl::CreateStorageBuffer(const void* data, size_t size, BufferStorageFlags storageFlags, std::string_view name)
//{
//	gl::BufferStorageId id;
//	glCreateBuffers(1, &id.id);
//
//	id.size = roundUp(size, 16);
//	id.storageFlags = storageFlags;
//
//	GLbitfield glflags = detail::BufferStorageFlagsToGL(storageFlags);
//	glNamedBufferStorage(id, (GLsizeiptr)id.size, data, glflags);
//
//	if (storageFlags & BufferStorageFlag::MapMemory)
//	{
//		// GL_MAP_UNSYNCHRONIZED_BIT should be used if the user can map and unmap buffers at their own will
//		constexpr GLenum access = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
//		id.mappedMemory = glMapNamedBufferRange(id, 0, (GLsizeiptr)size, access);
//	}
//
//	if (!name.empty())
//	{
//		glObjectLabel(GL_BUFFER, id, static_cast<GLsizei>(name.length()), name.data());
//	}
//
//	return id;
//}
//=============================================================================
void gl::UpdateData(BufferStorageId id, ByteView data, size_t destOffsetBytes)
{
	UpdateData(id, data.data(), data.size_bytes(), destOffsetBytes);
}
//=============================================================================
void gl::UpdateData(BufferStorageId id, const void* data, size_t size, size_t offset)
{
	assert((id.storageFlags & BufferStorageFlag::DynamicStorage) &&
		"UpdateData can only be called on buffers created with the DynamicStorage flag");
	assert(size + offset <= id.size);
	glNamedBufferSubData(id, static_cast<GLuint>(offset), static_cast<GLuint>(size), data);
}
//=============================================================================
void gl::FillData(BufferStorageId id, const BufferFillInfo& clear)
{
	const auto actualSize = clear.size == WHOLE_BUFFER ? id.size : clear.size;
	assert(actualSize % 4 == 0 && "Size must be a multiple of 4 bytes");
	glClearNamedBufferSubData(id,
		GL_R32UI,
		(GLsizeiptr)clear.offset,
		(GLsizeiptr)actualSize,
		GL_RED_INTEGER,
		GL_UNSIGNED_INT,
		&clear.data);
}
//=============================================================================
void gl::Invalidate(BufferStorageId id)
{
	glInvalidateBufferData(id);
}
//=============================================================================
#pragma endregion
//=============================================================================
#pragma region [ Vertex Array ]
//=============================================================================
void gl::SetVertexAttrib(GLuint vao, GLuint attribIndex, GLint size, GLenum type, GLboolean normalized, GLuint relativeOffset, GLuint bindingIndex)
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
void gl::SetVertexAttrib(GLuint vao, const VertexAttributeRaw& attr)
{
	SetVertexAttrib(vao, attr.index, attr.size, attr.type, attr.normalized ? GL_TRUE : GL_FALSE, attr.relativeOffset, attr.bindingIndex);
}
//=============================================================================
void gl::SetVertexAttrib(GLuint vao, const std::vector<VertexAttributeRaw>& attributes)
{
	for (size_t i = 0; i < attributes.size(); i++)
	{
		SetVertexAttrib(vao, attributes[i]);
	}
}
//=============================================================================
gl::VertexArrayId gl::CreateVertexArray()
{
	gl::VertexArrayId vao;
	glCreateVertexArrays(1, &vao.id);
	return vao;
}
//=============================================================================
gl::VertexArrayId gl::CreateVertexArray(const std::vector<VertexAttributeRaw>& attributes)
{
	gl::VertexArrayId vao = CreateVertexArray();
	SetVertexAttrib(vao, attributes);
	return vao;
}
//=============================================================================
gl::VertexArrayId gl::CreateVertexArray(gl::BufferId vbo, size_t vertexSize, const std::vector<VertexAttributeRaw>& attributes)
{
	return CreateVertexArray(vbo, { 0 }, vertexSize, attributes);
}
//=============================================================================
gl::VertexArrayId gl::CreateVertexArray(gl::BufferId vbo, gl::BufferId ibo, size_t vertexSize, const std::vector<VertexAttributeRaw>& attributes)
{
	gl::VertexArrayId vao = CreateVertexArray(attributes);
	SetVertexBuffer(vao, vbo, 0, 0, (GLsizei)vertexSize);
	SetIndexBuffer(vao, ibo);
	return vao;
}
//=============================================================================
//gl::VertexArrayId gl::CreateVertexArray(const VertexInputState& inputState)
//{
//	auto inputHash = vertexInputStateHash(inputState);
//	if (auto it = vertexArrayCache.find(inputHash); it != vertexArrayCache.end())
//	{
//		return it->second;
//	}
//
//	gl::VertexArrayId id;
//	glCreateVertexArrays(1, &id.id);
//
//	for (uint32_t i = 0; i < inputState.vertexBindingDescriptions.size(); i++)
//	{
//		const auto& desc = inputState.vertexBindingDescriptions[i];
//		glEnableVertexArrayAttrib(id, desc.location);
//		glVertexArrayAttribBinding(id, desc.location, desc.binding);
//
//		auto type = detail::FormatToTypeGL(desc.format);
//		auto size = detail::FormatToSizeGL(desc.format);
//		auto normalized = detail::IsFormatNormalizedGL(desc.format);
//		auto internalType = detail::FormatToFormatClass(desc.format);
//		switch (internalType)
//		{
//		case GlFormatClass::Float: glVertexArrayAttribFormat(id, desc.location, size, type, normalized, desc.offset); break;
//		case GlFormatClass::Int:   glVertexArrayAttribIFormat(id, desc.location, size, type, desc.offset); break;
//		case GlFormatClass::Long:  glVertexArrayAttribLFormat(id, desc.location, size, type, desc.offset); break;
//		default: assert(0);
//		}
//	}
//
//	return vertexArrayCache.insert({ inputHash, id }).first->second;
//}
//=============================================================================
void gl::SetVertexBuffer(VertexArrayId id, BufferId vbo, GLuint bindingindex, GLintptr offset, GLsizei stride)
{
	// TODO: возможно кеширование
	if (IsValid(vbo)) glVertexArrayVertexBuffer(id, bindingindex, vbo, offset, stride);
}
//=============================================================================
void gl::SetIndexBuffer(VertexArrayId id, BufferId ibo)
{
	if (IsValid(ibo)) glVertexArrayElementBuffer(id, ibo);
}
//=============================================================================
void gl::Bind(VertexArrayId id)
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
gl::Texture2DId gl::CreateTexture2D(GLenum internalFormat, GLsizei width, GLsizei height, void* data, const TextureParameter& param)
{
	const int numMipmaps = param.genMipMap ? getNumMipMapLevels2D(width, height) : 1;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	gl::Texture2DId texture;
	glCreateTextures(GL_TEXTURE_2D, 1, &texture.id);
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
		imgData[i * 3 + 0] = imgData[i * 3 + 1] = imgData[i * 3 + 2] = uint8_t(0xFF * ((row + col) % 2));
	}

	if (width) *width = w;
	if (height) *height = h;
	if (nrChannels) *nrChannels = 3;

	return imgData;
}
//=============================================================================
gl::Texture2DId gl::LoadTexture2D(const char* texturePath, bool flipVertical, const TextureParameter& param)
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

	gl::Texture2DId texture = CreateTexture2D(internalFormat, width, height, data, param);

	stbi_image_free(data);
	return texture;
}
//=============================================================================
gl::Texture2DId gl::LoadTexture2DHDR(const char* texturePath, bool flipVertical, const TextureParameter& param)
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
	gl::Texture2DId texture = CreateTexture2D(GL_RGB32F, width, height, data, param);
	stbi_image_free(data);
	return texture;
}
//=============================================================================
gl::TextureCubeId gl::LoadCubeMap(const std::vector<std::string>& files, const std::string& directory)
{
	// TODO: возможность настроить через TextureParameter

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	gl::TextureCubeId texture;
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &texture.id);

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
void gl::SetSubImage(Texture1DId id, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels)
{
	glTextureSubImage1D(id, level, xoffset, width, format, type, pixels);
}
//=============================================================================
void gl::SetSubImage(Texture2DId id, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
	glTextureSubImage2D(id, level, xoffset, yoffset, width, height, format, type, pixels);
}
//=============================================================================
void gl::SetSubImage(Texture3DId id, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels)
{
	glTextureSubImage3D(id, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}
//=============================================================================
void gl::SetCompressedSubImage(Texture1DId id, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data)
{
	glCompressedTextureSubImage1D(id, level, xoffset, width, format, imageSize, data);
}
//=============================================================================
void gl::SetCompressedSubImage(Texture2DId id, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data)
{
	glCompressedTextureSubImage2D(id, level, xoffset, yoffset, width, height, format, imageSize, data);
}
//=============================================================================
void gl::SetCompressedSubImage(Texture3DId id, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data)
{
	glCompressedTextureSubImage3D(id, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}
//=============================================================================
void gl::CopySubImage(Texture1DId id, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
	glCopyTextureSubImage1D(id, level, xoffset, x, y, width);
}
//=============================================================================
void gl::CopySubImage(Texture2DId id, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	glCopyTextureSubImage2D(id, level, xoffset, yoffset, x, y, width, height);
}
//=============================================================================
void gl::CopySubImage(Texture3DId id, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	glCopyTextureSubImage3D(id, level, xoffset, yoffset, zoffset, x, y, width, height);
}
//=============================================================================
void gl::Bind(GLuint unit, Texture1DId id)
{
	// кешировать
	glBindTextureUnit(unit, id);
}
//=============================================================================
void gl::Bind(GLuint unit, Texture2DId id)
{
	// кешировать
	glBindTextureUnit(unit, id);
}
//=============================================================================
void gl::Bind(GLuint unit, Texture3DId id)
{
	// кешировать
	glBindTextureUnit(unit, id);
}
//=============================================================================
void gl::Bind(GLuint unit, TextureCubeId id)
{
	// кешировать
	glBindTextureUnit(unit, id);
}
//=============================================================================
#pragma endregion
//=============================================================================
#pragma region [ Sampler ]
//=============================================================================
gl::SamplerId gl::CreateSampler(const SamplerState& createInfo)
{
	if (auto it = samplerCache.find(createInfo); it != samplerCache.end())
		return it->second;

	SamplerId id;
	glCreateSamplers(1, &id.id);

	glSamplerParameteri(id, GL_TEXTURE_COMPARE_MODE, createInfo.compareEnable ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE);
	glSamplerParameteri(id, GL_TEXTURE_COMPARE_FUNC, (GLint)detail::EnumToGL(createInfo.compareOp));

	GLint magFilter = (GLint)detail::EnumToGL(createInfo.magFilter);
	glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, magFilter);

	GLint minFilter = (GLint)detail::EnumToGL(createInfo.minFilter);
	glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, minFilter);

	glSamplerParameteri(id, GL_TEXTURE_WRAP_S, detail::EnumToGL(createInfo.addressModeU));
	glSamplerParameteri(id, GL_TEXTURE_WRAP_T, detail::EnumToGL(createInfo.addressModeV));
	glSamplerParameteri(id, GL_TEXTURE_WRAP_R, detail::EnumToGL(createInfo.addressModeW));

	// TODO: determine whether int white values should be 1 or 255
	switch (createInfo.borderColor)
	{
	case BorderColor::FloatTransparentBlack:
	{
		constexpr GLfloat color[4]{ 0, 0, 0, 0 };
		glSamplerParameterfv(id, GL_TEXTURE_BORDER_COLOR, color);
		break;
	}
	case BorderColor::IntTransparentBlack:
	{
		constexpr GLint color[4]{ 0, 0, 0, 0 };
		glSamplerParameteriv(id, GL_TEXTURE_BORDER_COLOR, color);
		break;
	}
	case BorderColor::FloatOpaqueBlack:
	{
		constexpr GLfloat color[4]{ 0, 0, 0, 1 };
		glSamplerParameterfv(id, GL_TEXTURE_BORDER_COLOR, color);
		break;
	}
	case BorderColor::IntOpaqueBlack:
	{
		// constexpr GLint color[4]{ 0, 0, 0, 255 };
		constexpr GLint color[4]{ 0, 0, 0, 1 };
		glSamplerParameteriv(id, GL_TEXTURE_BORDER_COLOR, color);
		break;
	}
	case BorderColor::FloatOpaqueWhite:
	{
		constexpr GLfloat color[4]{ 1, 1, 1, 1 };
		glSamplerParameterfv(id, GL_TEXTURE_BORDER_COLOR, color);
		break;
	}
	case BorderColor::IntOpaqueWhite:
	{
		// constexpr GLint color[4]{ 255, 255, 255, 255 };
		constexpr GLint color[4]{ 1, 1, 1, 1 };
		glSamplerParameteriv(id, GL_TEXTURE_BORDER_COLOR, color);
		break;
	}
	default: assert(0); break;
	}

	glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY, static_cast<GLfloat>(detail::EnumToGL(createInfo.anisotropy)));

	glSamplerParameterf(id, GL_TEXTURE_LOD_BIAS, createInfo.lodBias);
	glSamplerParameterf(id, GL_TEXTURE_MIN_LOD, createInfo.minLod);
	glSamplerParameterf(id, GL_TEXTURE_MAX_LOD, createInfo.maxLod);

	return samplerCache.insert({ createInfo, SamplerId(id) }).first->second;
}
//=============================================================================
void gl::Bind(GLuint unit, SamplerId sampler)
{
	// TODO: кеширование
	glBindSampler(unit, sampler);
}
//=============================================================================
void gl::Bind(GLuint unit, Texture2DId texture, SamplerId sampler)
{
	Bind(unit, texture);
	Bind(unit, sampler);
}
//=============================================================================
#pragma endregion
//=============================================================================
#pragma region [ RenderBuffer ]
//=============================================================================
gl::RenderBufferId gl::CreateRenderBuffer(GLenum internalFormat, GLsizei width, GLsizei height)
{
	RenderBufferId id;
	glCreateRenderbuffers(1, &id.id);
	glNamedRenderbufferStorage(id, internalFormat, width, height);
	return id;
}
//=============================================================================
gl::RenderBufferId gl::CreateRenderBuffer(GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height)
{
	RenderBufferId id;
	glCreateRenderbuffers(1, &id.id);
	glNamedRenderbufferStorageMultisample(id, samples, internalFormat, width, height);
	return id;
}
//=============================================================================
#pragma endregion
//=============================================================================
#pragma region [ FrameBuffer ]
//=============================================================================
GLuint gl::CreateColorBuffer2D(int width, int height, GLenum formatColor)
{
	TextureParameter param = defaultTextureParameter2D;
	param.wrap = GL_CLAMP_TO_EDGE;
	return gl::CreateTexture2D(formatColor, width, height, nullptr, param);
}
//=============================================================================
GLuint gl::CreateDepthBuffer2D(int width, int height, GLenum formatDepth)
{
	TextureParameter param = {};
	param.minFilter = GL_NEAREST;
	param.magFilter = GL_NEAREST;
	param.wrap = GL_CLAMP_TO_BORDER;
	param.genMipMap = false;
	param.dataType = GL_FLOAT;

	GLuint texture = gl::CreateTexture2D(formatDepth, width, height, nullptr, param);
	constexpr GLfloat border[]{ 1.0f, 1.0f, 1.0f, 1.0f };
	glTextureParameterfv(texture, GL_TEXTURE_BORDER_COLOR, border);

	return texture;
}
//=============================================================================
gl::FrameBufferId gl::CreateFrameBuffer2D(GLuint colorBuffer, GLuint depthBuffer)
{
	gl::FrameBufferId framebuffer;
	glCreateFramebuffers(1, &framebuffer.id);

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
gl::FrameBufferId gl::CreateFrameBuffer2D(Texture2DId colorBuffer, Texture2DId depthBuffer)
{
	gl::FrameBufferId framebuffer;
	glCreateFramebuffers(1, &framebuffer.id);

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
gl::FrameBufferId gl::CreateFrameBuffer2D(Texture2DId colorBuffer, RenderBufferId depthBuffer)
{
	gl::FrameBufferId framebuffer;
	glCreateFramebuffers(1, &framebuffer.id);

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
void gl::SetDrawBuffer(FrameBufferId fbo, GLenum buffer)
{
	glNamedFramebufferDrawBuffer(fbo, buffer);
}
//=============================================================================
void gl::SetDrawBuffers(FrameBufferId fbo, GLsizei size, const GLenum* buffers)
{
	glNamedFramebufferDrawBuffers(fbo, size, buffers);
}
//=============================================================================
void gl::Invalidate(FrameBufferId fbo, GLsizei numAttachments, const GLenum* attachments)
{
	glInvalidateNamedFramebufferData(fbo, numAttachments, attachments);
}
//=============================================================================
void gl::InvalidateSubData(FrameBufferId fbo, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
	glInvalidateNamedFramebufferSubData(fbo, numAttachments, attachments, x, y, width, height);
}
//=============================================================================
void gl::SetFrameBuffer(FrameBufferId fbo)
{
	if (currentFBO != fbo)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		currentFBO = fbo;
	}
}
//=============================================================================
void gl::SetFrameBuffer(FrameBufferId fbo, int width, int height, GLbitfield clearMask)
{
	SetFrameBuffer(fbo);
	glViewport(0, 0, width, height);
	glClear(clearMask);
}
//=============================================================================
#pragma endregion
//=============================================================================