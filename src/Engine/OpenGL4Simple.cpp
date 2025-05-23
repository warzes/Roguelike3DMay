#include "stdafx.h"
#include "OpenGL4Simple.h"
#include "Log.h"
#include "OpenGL4DeviceProperties.h"
#include "Hash.h"
//=============================================================================
#pragma region [ Hash ]
//=============================================================================
inline size_t vertexInputStateHash(const gl4::VertexInputState& k)
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
template<>
struct std::hash<gl4::SamplerState>
{
	std::size_t operator()(const gl4::SamplerState& k) const noexcept
	{
		auto rtup = std::make_tuple(k.minFilter,
			k.magFilter,
			k.addressModeU,
			k.addressModeV,
			k.addressModeW,
			k.borderColor,
			k.anisotropy,
			k.compareEnable,
			k.compareOp,
			k.lodBias,
			k.minLod,
			k.maxLod);
		return detail::hashing::hash<decltype(rtup)>{}(rtup);
	}
};
//=============================================================================
#pragma endregion
//=============================================================================
#pragma region [ Local variables ]
//=============================================================================
struct GraphicsPipelineCache final
{
	std::string_view   debugName;

	gl4::InputAssemblyState inputAssemblyState;
	gl4::TessellationState  tessellationState;
	gl4::RasterizationState rasterizationState;
	gl4::MultisampleState   multisampleState;
	gl4::DepthState         depthState;
	gl4::StencilState       stencilState;
	gl4::ColorBlendState    colorBlendState;

	bool valid{ false };

	GraphicsPipelineCache& operator=(const GraphicsPipelineCache&) = default;
	GraphicsPipelineCache& operator=(const gl4::GraphicsPipelineId& p)
	{
		valid = true;

		debugName = p.debugName;
		inputAssemblyState = p.inputAssemblyState;
		tessellationState = p.tessellationState;
		rasterizationState = p.rasterizationState;
		multisampleState = p.multisampleState;
		depthState = p.depthState;
		stencilState = p.stencilState;
		colorBlendState = p.colorBlendState;

		return *this;
	}

	bool operator==(const gl4::GraphicsPipelineId& p) const noexcept
	{
		return (
			valid == p.valid &&
			debugName == p.debugName &&
			inputAssemblyState == p.inputAssemblyState &&
			tessellationState == p.tessellationState &&
			rasterizationState == p.rasterizationState &&
			multisampleState == p.multisampleState &&
			depthState == p.depthState &&
			stencilState == p.stencilState &&
			colorBlendState == p.colorBlendState
			);
	}
};
//=============================================================================
namespace
{
	constexpr int MAX_COLOR_ATTACHMENTS = 8;

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

	GraphicsPipelineCache lastGraphicsPipeline{};
	gl4::ComputePipelineId lastComputePipeline{};
	gl4::ShaderProgramId lastProgram; // шейдер отделен от pipeline так как есть еще вычислительный шейдер и долно сбрасывать

	// Potentially used for state deduplication.
	gl4::VertexArrayId currentVao{ 0 };
	gl4::FrameBufferId currentFBO{ 0 };

	// Currently unused (and probably shouldn't be used)
	const gl4::RenderInfo* lastRenderInfo{};

	// These can be set at the start of rendering, so they need to be tracked separately from the other pipeline state.
	std::array<gl4::ColorComponentFlags, MAX_COLOR_ATTACHMENTS> lastColorMask = {};
	bool lastDepthMask = true;
	uint32_t lastStencilMask[2] = { static_cast<uint32_t>(-1), static_cast<uint32_t>(-1) };
	bool initViewport = true;
	gl4::Viewport lastViewport = {};
	Rect2D lastScissor = {};
	bool scissorEnabled = false;

	// These persist until another Pipeline is bound.
	// They are not used for state deduplication, as they are arguments for GL draw calls.
	gl4::PrimitiveTopology currentTopology{};
	gl4::IndexType currentIndexType{};

	std::unordered_map<size_t, gl4::VertexArrayId> vertexArrayCache;
	std::unordered_map<gl4::SamplerState, gl4::SamplerId> samplerCache;
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

	lastGraphicsPipeline.valid = false;
	lastComputePipeline.valid = false;
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
	for (auto& [_, vao] : vertexArrayCache) { gl4::Destroy(vao); }
	vertexArrayCache.clear();
	for (auto& [_, sampler] : samplerCache) { gl4::Destroy(sampler); }
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
	gl4::ShaderProgramId program{ glCreateProgram() };
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
	gl4::ShaderProgramId program{ glCreateProgram() };

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
	glCreateBuffers(1, &buffer.id);
	glNamedBufferData(buffer, size, data, usage);
	return buffer;
}
//=============================================================================
gl4::BufferId gl4::CreateBufferStorage(GLbitfield flags, GLsizeiptr size, const void* data)
{
	gl4::BufferId buffer;
	glCreateBuffers(1, &buffer.id);
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
#pragma region [ BufferStorage ]
//=============================================================================
constexpr size_t roundUp(size_t numberToRoundUp, size_t multipleOf)
{
	assert(multipleOf);
	return ((numberToRoundUp + multipleOf - 1) / multipleOf) * multipleOf;
}
//=============================================================================
gl4::BufferStorageId gl4::CreateStorageBuffer(size_t size, BufferStorageFlags storageFlags, std::string_view name)
{
	return CreateStorageBuffer(nullptr, size, storageFlags, name);
}
//=============================================================================
gl4::BufferStorageId gl4::CreateStorageBuffer(TriviallyCopyableByteSpan data, BufferStorageFlags storageFlags, std::string_view name)
{
	return CreateStorageBuffer(data.data(), data.size_bytes(), storageFlags, name);
}
//=============================================================================
gl4::BufferStorageId gl4::CreateStorageBuffer(const void* data, size_t size, BufferStorageFlags storageFlags, std::string_view name)
{
	gl4::BufferStorageId id;
	glCreateBuffers(1, &id.id);

	id.size = roundUp(size, 16);
	id.storageFlags = storageFlags;

	GLbitfield glflags = BufferStorageFlagsToGL(storageFlags);
	glNamedBufferStorage(id, id.size, data, glflags);

	if (storageFlags & BufferStorageFlag::MAP_MEMORY)
	{
		// GL_MAP_UNSYNCHRONIZED_BIT should be used if the user can map and unmap buffers at their own will
		constexpr GLenum access = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		id.mappedMemory = glMapNamedBufferRange(id, 0, size, access);
	}

	if (!name.empty())
	{
		glObjectLabel(GL_BUFFER, id, static_cast<GLsizei>(name.length()), name.data());
	}

	return id;
}
//=============================================================================
void gl4::UpdateData(BufferStorageId id, TriviallyCopyableByteSpan data, size_t destOffsetBytes)
{
	UpdateData(id, data.data(), data.size_bytes(), destOffsetBytes);
}
//=============================================================================
void gl4::UpdateData(BufferStorageId id, const void* data, size_t size, size_t offset)
{
	assert((id.storageFlags & BufferStorageFlag::DYNAMIC_STORAGE) &&
		"UpdateData can only be called on buffers created with the DYNAMIC_STORAGE flag");
	assert(size + offset <= id.size);
	glNamedBufferSubData(id, static_cast<GLuint>(offset), static_cast<GLuint>(size), data);
}
//=============================================================================
void gl4::FillData(BufferStorageId id, const BufferFillInfo& clear)
{
	const auto actualSize = clear.size == WHOLE_BUFFER ? id.size : clear.size;
	assert(actualSize % 4 == 0 && "Size must be a multiple of 4 bytes");
	glClearNamedBufferSubData(id,
		GL_R32UI,
		clear.offset,
		actualSize,
		GL_RED_INTEGER,
		GL_UNSIGNED_INT,
		&clear.data);
}
//=============================================================================
void gl4::Invalidate(BufferStorageId id)
{
	glInvalidateBufferData(id);
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
void gl4::SetVertexAttrib(GLuint vao, const VertexAttributeRaw& attr)
{
	SetVertexAttrib(vao, attr.index, attr.size, attr.type, attr.normalized ? GL_TRUE : GL_FALSE, attr.relativeOffset, attr.bindingIndex);
}
//=============================================================================
void gl4::SetVertexAttrib(GLuint vao, const std::vector<VertexAttributeRaw>& attributes)
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
	glCreateVertexArrays(1, &vao.id);
	return vao;
}
//=============================================================================
gl4::VertexArrayId gl4::CreateVertexArray(const std::vector<VertexAttributeRaw>& attributes)
{
	gl4::VertexArrayId vao = CreateVertexArray();
	SetVertexAttrib(vao, attributes);
	return vao;
}
//=============================================================================
gl4::VertexArrayId gl4::CreateVertexArray(gl4::BufferId vbo, size_t vertexSize, const std::vector<VertexAttributeRaw>& attributes)
{
	return CreateVertexArray(vbo, { 0 }, vertexSize, attributes);
}
//=============================================================================
gl4::VertexArrayId gl4::CreateVertexArray(gl4::BufferId vbo, gl4::BufferId ibo, size_t vertexSize, const std::vector<VertexAttributeRaw>& attributes)
{
	gl4::VertexArrayId vao = CreateVertexArray(attributes);
	SetVertexBuffer(vao, vbo, 0, 0, vertexSize);
	SetIndexBuffer(vao, ibo);
	return vao;
}
//=============================================================================
gl4::VertexArrayId gl4::CreateVertexArray(const VertexInputState& inputState)
{
	auto inputHash = vertexInputStateHash(inputState);
	if (auto it = vertexArrayCache.find(inputHash); it != vertexArrayCache.end())
	{
		return it->second;
	}

	gl4::VertexArrayId id;
	glCreateVertexArrays(1, &id.id);

	for (uint32_t i = 0; i < inputState.vertexBindingDescriptions.size(); i++)
	{
		const auto& desc = inputState.vertexBindingDescriptions[i];
		glEnableVertexArrayAttrib(id, desc.location);
		glVertexArrayAttribBinding(id, desc.location, desc.binding);

		auto type = FormatToTypeGL(desc.format);
		auto size = FormatToSizeGL(desc.format);
		auto normalized = IsFormatNormalizedGL(desc.format);
		auto internalType = FormatToFormatClass(desc.format);
		switch (internalType)
		{
		case GlFormatClass::Float: glVertexArrayAttribFormat(id, desc.location, size, type, normalized, desc.offset); break;
		case GlFormatClass::Int:   glVertexArrayAttribIFormat(id, desc.location, size, type, desc.offset); break;
		case GlFormatClass::Long:  glVertexArrayAttribLFormat(id, desc.location, size, type, desc.offset); break;
		default: assert(0);
		}
	}

	return vertexArrayCache.insert({ inputHash, id }).first->second;
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
#pragma region [ (NEW) Texture ]
//=============================================================================
gl4::TextureId gl4::CreateTexture(const TextureCreateInfo& createInfo, std::string_view name)
{
	gl4::TextureId id;
	id.info = createInfo;
	glCreateTextures(EnumToGL(createInfo.imageType), 1, &id.id);

	switch (createInfo.imageType)
	{
	case ImageType::Tex1D:
		glTextureStorage1D(id, createInfo.mipLevels, FormatToGL(createInfo.format), createInfo.extent.width);
		break;
	case ImageType::Tex2D:
		glTextureStorage2D(id, createInfo.mipLevels, FormatToGL(createInfo.format), createInfo.extent.width, createInfo.extent.height);
		break;
	case ImageType::Tex3D:
		glTextureStorage3D(id, createInfo.mipLevels, FormatToGL(createInfo.format), createInfo.extent.width, createInfo.extent.height, createInfo.extent.depth);
		break;
	case ImageType::Tex1DArray:
		glTextureStorage2D(id, createInfo.mipLevels, FormatToGL(createInfo.format), createInfo.extent.width, createInfo.arrayLayers);
		break;
	case ImageType::Tex2DArray:
		glTextureStorage3D(id, createInfo.mipLevels, FormatToGL(createInfo.format), createInfo.extent.width, createInfo.extent.height, createInfo.arrayLayers);
		break;
	case ImageType::TexCubemap:
		glTextureStorage2D(id, createInfo.mipLevels, FormatToGL(createInfo.format), createInfo.extent.width, createInfo.extent.height);
		break;
	case ImageType::TexCubemapArray:
		glTextureStorage3D(id, createInfo.mipLevels, FormatToGL(createInfo.format), createInfo.extent.width, createInfo.extent.height, createInfo.arrayLayers);
		break;
	case ImageType::Tex2DMultisample:
		glTextureStorage2DMultisample(id, EnumToGL(createInfo.sampleCount), FormatToGL(createInfo.format), createInfo.extent.width, createInfo.extent.height, GL_TRUE);
		break;
	case ImageType::Tex2DMultisampleArray:
		glTextureStorage3DMultisample(id, EnumToGL(createInfo.sampleCount), FormatToGL(createInfo.format), createInfo.extent.width, createInfo.extent.height, createInfo.arrayLayers, GL_TRUE);
		break;
	default: assert(0); break;
	}

	if (!name.empty())
		glObjectLabel(GL_TEXTURE, id, static_cast<GLsizei>(name.length()), name.data());

	return id;
}
//=============================================================================
gl4::TextureId gl4::CreateTexture2D(Extent2D size, Format format, std::string_view name)
{
	TextureCreateInfo createInfo{
			.imageType = ImageType::Tex2D,
			.format = format,
			.extent = {size.width, size.height, 1},
			.mipLevels = 1,
			.arrayLayers = 1,
			.sampleCount = SampleCount::Samples1,
	};
	return CreateTexture(createInfo, name);
}
//=============================================================================
gl4::TextureId gl4::CreateTexture2DMip(Extent2D size, Format format, uint32_t mipLevels, std::string_view name)
{
	TextureCreateInfo createInfo{
	.imageType = ImageType::Tex2D,
	.format = format,
	.extent = {size.width, size.height, 1},
	.mipLevels = mipLevels,
	.arrayLayers = 1,
	.sampleCount = SampleCount::Samples1,
	};
	return CreateTexture(createInfo, name);
}
//=============================================================================
void gl4::UpdateImage(TextureId id, const TextureUpdateInfo& info)
{
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	assert(!IsBlockCompressedFormat(id.info.format));
	GLenum format{};
	if (info.format == UploadFormat::INFER_FORMAT)
		format = EnumToGL(FormatToUploadFormat(id.info.format));
	else
		format = EnumToGL(info.format);

	GLenum type{};
	if (info.type == UploadType::INFER_TYPE)
		type = FormatToTypeGL(id.info.format);
	else
		type = EnumToGL(info.type);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, info.rowLength);
	glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, info.imageHeight);

	switch (ImageTypeToDimension(id.info.imageType))
	{
	case 1:
		glTextureSubImage1D(
			id,
			info.level,
			info.offset.x,
			info.extent.width,
			format,
			type,
			info.pixels);
		break;
	case 2:
		glTextureSubImage2D(
			id,
			info.level,
			info.offset.x,
			info.offset.y,
			info.extent.width,
			info.extent.height,
			format,
			type,
			info.pixels);
		break;
	case 3:
		glTextureSubImage3D(
			id,
			info.level,
			info.offset.x,
			info.offset.y,
			info.offset.z,
			info.extent.width,
			info.extent.height,
			info.extent.depth,
			format,
			type,
			info.pixels);
		break;
	}
}
//=============================================================================
inline uint64_t getBlockCompressedImageSize(gl4::Format format, uint32_t width, uint32_t height, uint32_t depth)
{
	assert(gl4::IsBlockCompressedFormat(format));

	// BCn formats store 4x4 blocks of pixels, even if the dimensions aren't a multiple of 4
	// We round up to the nearest multiple of 4 for width and height, but not depth, since
	// 3D BCn images are just multiple 2D images stacked
	width = (width + 4 - 1) & -4;
	height = (height + 4 - 1) & -4;

	switch (format)
	{
		// BC1 and BC4 store 4x4 blocks with 64 bits (8 bytes)
	case gl4::Format::BC1_RGB_UNORM:
	case gl4::Format::BC1_RGBA_UNORM:
	case gl4::Format::BC1_RGB_SRGB:
	case gl4::Format::BC1_RGBA_SRGB:
	case gl4::Format::BC4_R_UNORM:
	case gl4::Format::BC4_R_SNORM:
		return width * height * depth / 2;

	// BC3, BC5, BC6, and BC7 store 4x4 blocks with 128 bits (16 bytes)
	case gl4::Format::BC2_RGBA_UNORM:
	case gl4::Format::BC2_RGBA_SRGB:
	case gl4::Format::BC3_RGBA_UNORM:
	case gl4::Format::BC3_RGBA_SRGB:
	case gl4::Format::BC5_RG_UNORM:
	case gl4::Format::BC5_RG_SNORM:
	case gl4::Format::BC6H_RGB_UFLOAT:
	case gl4::Format::BC6H_RGB_SFLOAT:
	case gl4::Format::BC7_RGBA_UNORM:
	case gl4::Format::BC7_RGBA_SRGB:
		return width * height * depth;
	default: assert(0); return 0;
	}
}
//=============================================================================
void gl4::UpdateCompressedImage(TextureId id, const CompressedTextureUpdateInfo& info)
{
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	assert(IsBlockCompressedFormat(id.info.format));
	const GLenum format = FormatToGL(id.info.format);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);

	switch (ImageTypeToDimension(id.info.imageType))
	{
	case 2:
		glCompressedTextureSubImage2D(
			id,
			info.level,
			info.offset.x,
			info.offset.y,
			info.extent.width,
			info.extent.height,
			format,
			static_cast<uint32_t>(getBlockCompressedImageSize(id.info.format, info.extent.width, info.extent.height, 1)),
			info.data);
		break;
	case 3:
		glCompressedTextureSubImage3D(
			id,
			info.level,
			info.offset.x,
			info.offset.y,
			info.offset.z,
			info.extent.width,
			info.extent.height,
			info.extent.depth,
			format,
			static_cast<uint32_t>(getBlockCompressedImageSize(id.info.format, info.extent.width, info.extent.height, info.extent.depth)),
			info.data);
		break;
	default: assert(0);
	}
}
//=============================================================================
void gl4::ClearImage(TextureId id, const TextureClearInfo& info)
{
	// Infer format
	GLenum format{};
	if (info.format == UploadFormat::INFER_FORMAT)
		format = EnumToGL(FormatToUploadFormat(id.info.format));
	else
		format = EnumToGL(info.format);

	// Infer type
	GLenum type{};
	if (info.type == UploadType::INFER_TYPE) 
		type = FormatToTypeGL(id.info.format);
	else 
		type = EnumToGL(info.type);

	// Infer extent
	Extent3D extent = info.extent;
	if (extent == Extent3D{})
	{
		extent = id.info.extent >> info.level;
		extent.width = std::max(extent.width, 1u);
		extent.height = std::max(extent.height, 1u);
		extent.depth = std::max(extent.depth, 1u);
	}

	glClearTexSubImage(id,
		info.level,
		info.offset.x,
		info.offset.y,
		info.offset.z,
		extent.width,
		extent.height,
		extent.depth,
		format,
		type,
		info.data);
}
//=============================================================================
void gl4::GenMipmaps(TextureId id)
{
	glGenerateTextureMipmap(id);
}
//=============================================================================
uint64_t gl4::GetBindlessHandle(TextureId id, SamplerId sampler)
{
	assert(gDeviceProperties.features.bindlessTextures && "GL_ARB_bindless_texture is not supported");
	assert(id.bindlessHandle == 0 && "Texture already has bindless handle resident.");
	id.bindlessHandle = glGetTextureSamplerHandleARB(id, sampler);
	assert(id.bindlessHandle != 0 && "Failed to create texture sampler handle.");
	glMakeTextureHandleResidentARB(id.bindlessHandle);
	return id.bindlessHandle;
}
//=============================================================================
#pragma endregion
//=============================================================================
#pragma region [ TextureView ]
//=============================================================================
gl4::TextureViewId gl4::CreateTextureView(const TextureViewCreateInfo& viewInfo, TextureId texture, std::string_view name)
{
	gl4::TextureViewId id;
	id.info = viewInfo;
	glGenTextures(1, &id.id); // glCreateTextures does not work here
	glTextureView(id,
		EnumToGL(viewInfo.viewType),
		texture,
		FormatToGL(viewInfo.format),
		viewInfo.minLevel,
		viewInfo.numLevels,
		viewInfo.minLayer,
		viewInfo.numLayers);

	glTextureParameteri(id, GL_TEXTURE_SWIZZLE_R, EnumToGL(viewInfo.components.r));
	glTextureParameteri(id, GL_TEXTURE_SWIZZLE_G, EnumToGL(viewInfo.components.g));
	glTextureParameteri(id, GL_TEXTURE_SWIZZLE_B, EnumToGL(viewInfo.components.b));
	glTextureParameteri(id, GL_TEXTURE_SWIZZLE_A, EnumToGL(viewInfo.components.a));

	if (!name.empty())
		glObjectLabel(GL_TEXTURE, id, static_cast<GLsizei>(name.length()), name.data());

	return id;
}
//=============================================================================
gl4::TextureViewId gl4::CreateTextureView(TextureId texture, std::string_view name)
{
	TextureViewCreateInfo createInfo{
		.viewType = texture.info.imageType,
		.format = texture.info.format,
		.minLevel = 0,
		.numLevels = texture.info.mipLevels,
		.minLayer = 0,
		.numLayers = texture.info.arrayLayers,
	};
	return CreateTextureView(createInfo, texture, name);
}
//=============================================================================
gl4::TextureViewId gl4::CreateSingleMipView(TextureId texture, uint32_t level)
{
	TextureViewCreateInfo createInfo{
		.viewType = texture.info.imageType,
		.format = texture.info.format,
		.minLevel = level,
		.numLevels = 1,
		.minLayer = 0,
		.numLayers = texture.info.arrayLayers,
	};
	return CreateTextureView(createInfo, texture);
}
//=============================================================================
gl4::TextureViewId gl4::CreateSingleLayerView(TextureId texture, uint32_t layer)
{
	TextureViewCreateInfo createInfo{
		.viewType = texture.info.imageType,
		.format = texture.info.format,
		.minLevel = 0,
		.numLevels = texture.info.mipLevels,
		.minLayer = layer,
		.numLayers = 1,
	};
	return CreateTextureView(createInfo, texture);
}
//=============================================================================
gl4::TextureViewId gl4::CreateFormatView(TextureId texture, Format newFormat)
{
	TextureViewCreateInfo createInfo{
		.viewType = texture.info.imageType,
		.format = newFormat,
		.minLevel = 0,
		.numLevels = texture.info.mipLevels,
		.minLayer = 0,
		.numLayers = texture.info.arrayLayers,
	};
	return CreateTextureView(createInfo, texture);
}
//=============================================================================
gl4::TextureViewId gl4::CreateSwizzleView(TextureId texture, ComponentMapping components)
{
	TextureViewCreateInfo createInfo{
		.viewType = texture.info.imageType,
		.format = texture.info.format,
		.components = components,
		.minLevel = 0,
		.numLevels = texture.info.mipLevels,
		.minLayer = 0,
		.numLayers = texture.info.arrayLayers,
	};
	return CreateTextureView(createInfo, texture);
}
//=============================================================================
#pragma endregion
//=============================================================================
#pragma region [ Sampler ]
//=============================================================================
gl4::SamplerId gl4::CreateSampler(const SamplerState& createInfo)
{
	if (auto it = samplerCache.find(createInfo); it != samplerCache.end())
		return it->second;

	SamplerId id;
	glCreateSamplers(1, &id.id);

	glSamplerParameteri(id, GL_TEXTURE_COMPARE_MODE, createInfo.compareEnable ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE);
	glSamplerParameteri(id, GL_TEXTURE_COMPARE_FUNC, EnumToGL(createInfo.compareOp));

	GLint magFilter = EnumToGL(createInfo.magFilter);
	glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, magFilter);

	GLint minFilter = EnumToGL(createInfo.minFilter);
	glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, minFilter);

	glSamplerParameteri(id, GL_TEXTURE_WRAP_S, EnumToGL(createInfo.addressModeU));
	glSamplerParameteri(id, GL_TEXTURE_WRAP_T, EnumToGL(createInfo.addressModeV));
	glSamplerParameteri(id, GL_TEXTURE_WRAP_R, EnumToGL(createInfo.addressModeW));

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

	glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY, static_cast<GLfloat>(EnumToGL(createInfo.anisotropy)));

	glSamplerParameterf(id, GL_TEXTURE_LOD_BIAS, createInfo.lodBias);
	glSamplerParameterf(id, GL_TEXTURE_MIN_LOD, createInfo.minLod);
	glSamplerParameterf(id, GL_TEXTURE_MAX_LOD, createInfo.maxLod);

	return samplerCache.insert({ createInfo, SamplerId(id) }).first->second;
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
	glCreateRenderbuffers(1, &id.id);
	glNamedRenderbufferStorage(id, internalFormat, width, height);
	return id;
}
//=============================================================================
gl4::RenderBufferId gl4::CreateRenderBuffer(GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height)
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
gl4::FrameBufferId gl4::CreateFrameBuffer2D(Texture2DId colorBuffer, Texture2DId depthBuffer)
{
	gl4::FrameBufferId framebuffer;
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
gl4::FrameBufferId gl4::CreateFrameBuffer2D(Texture2DId colorBuffer, RenderBufferId depthBuffer)
{
	gl4::FrameBufferId framebuffer;
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
#pragma region [ (NEW) FrameBuffer ]
//=============================================================================
gl4::FrameBufferId gl4::CreateFrameBuffer(const FrameBufferCreateInfo& createInfo)
{
	FrameBufferId id;
	glCreateFramebuffers(1, &id.id);
	
	std::vector<GLenum> drawBuffers;
	for (size_t i = 0; i < createInfo.colorAttachments.size(); i++)
	{
		const auto& attachment = createInfo.colorAttachments[i];
		glNamedFramebufferTexture(id, static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i), attachment.id, 0);
		drawBuffers.push_back(static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i));
	}
	glNamedFramebufferDrawBuffers(id, static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());

	if (createInfo.depthAttachment && createInfo.stencilAttachment &&
		createInfo.depthAttachment == createInfo.stencilAttachment)
	{
		glNamedFramebufferTexture(id, GL_DEPTH_STENCIL_ATTACHMENT, createInfo.depthAttachment->id, 0);
	}
	else
	{
		if (createInfo.depthAttachment)
		{
			glNamedFramebufferTexture(id, GL_DEPTH_ATTACHMENT, createInfo.depthAttachment->id, 0);
		}

		if (createInfo.stencilAttachment)
		{
			glNamedFramebufferTexture(id, GL_STENCIL_ATTACHMENT, createInfo.stencilAttachment->id, 0);
		}
	}

	return id;
}
//=============================================================================
#pragma endregion
//=============================================================================
#pragma region [ GraphicsPipeline ]
//=============================================================================
inline std::vector<std::pair<std::string, uint32_t>> reflectProgram(GLuint program, GLenum interface)
{
	GLint numActiveResources{};
	glGetProgramInterfaceiv(program, interface, GL_ACTIVE_RESOURCES, &numActiveResources);

	GLint maxNameLength{};
	glGetProgramInterfaceiv(program, interface, GL_MAX_NAME_LENGTH, &maxNameLength);

	auto reflected = std::vector<std::pair<std::string, uint32_t>>(numActiveResources, { std::string(maxNameLength, '\0'), 0 });

	for (GLint i = 0; i < numActiveResources; i++)
	{
		// Reflect the name of the resource
		glGetProgramResourceName(program,
			interface,
			i,
			static_cast<GLsizei>(reflected[i].first.size()),
			nullptr,
			reflected[i].first.data());

		// Reflect the value or binding of the resource (for samplers and images, these are essentially bindings as well)
		if (interface == GL_UNIFORM)
		{
			auto location = glGetProgramResourceLocation(program, interface, reflected[i].first.c_str());

			// Variables inside uniform blocks will also be iterated here. Their location will be -1, so we can skip them this way.
			if (location != -1)
			{
				// Wrong if the uniform isn't an integer (e.g., sampler or image). OK, default-block uniforms are not supported otherwise.
				GLint binding{ -1 };
				glGetUniformiv(program, location, &binding);
				reflected[i].second = static_cast<uint32_t>(binding);
			}
			else
			{
				reflected[i].first = "";
			}
		}
		else if (interface == GL_UNIFORM_BLOCK || interface == GL_SHADER_STORAGE_BLOCK)
		{
			GLint binding{ -1 };
			constexpr GLenum property = GL_BUFFER_BINDING;
			glGetProgramResourceiv(program, interface, i, 1, &property, 1, nullptr, &binding);
			reflected[i].second = static_cast<uint32_t>(binding);
		}
		else
		{
			assert(0);
		}
	}

	auto it = std::remove_if(reflected.begin(), reflected.end(), [](const auto& pair) { return pair.first.empty(); });
	reflected.erase(it, reflected.end());

	return reflected;
}
//=============================================================================
gl4::GraphicsPipelineId gl4::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)
{
	assert(!createInfo.vertexShader.empty() && "A graphics pipeline must at least have a vertex shader");
	if (!createInfo.tessellationControlShader.empty() || !createInfo.tessellationEvaluationShader.empty())
	{
		assert(!createInfo.tessellationControlShader.empty() && !createInfo.tessellationEvaluationShader.empty() && "Either both or neither tessellation shader can be present");
	}

	gl4::GraphicsPipelineId id;
	id.program = CreateShaderProgram(createInfo.vertexShader.data(), createInfo.fragmentShader.data());
	if (!id.program) return {};

	id.vao = CreateVertexArray(createInfo.vertexInputState);
	if (!id.vao) return {};


	if (!createInfo.debugName.empty())
	{
		glObjectLabel(GL_PROGRAM, id.program, static_cast<GLsizei>(createInfo.debugName.length()), createInfo.debugName.data());
	}

	id.debugName          = createInfo.debugName;
	id.inputAssemblyState = createInfo.inputAssemblyState;
	id.vertexInputState   = createInfo.vertexInputState;
	id.tessellationState  = createInfo.tessellationState;
	id.rasterizationState = createInfo.rasterizationState;
	id.multisampleState   = createInfo.multisampleState;
	id.depthState         = createInfo.depthState;
	id.stencilState       = createInfo.stencilState;
	id.colorBlendState    = createInfo.colorBlendState;

	id.uniformBlocks     = reflectProgram(id.program, GL_UNIFORM_BLOCK);
	id.storageBlocks     = reflectProgram(id.program, GL_SHADER_STORAGE_BLOCK);
	id.samplersAndImages = reflectProgram(id.program, GL_UNIFORM);

	id.valid = true;
	return id;
}
//=============================================================================
#pragma endregion
//=============================================================================
#pragma region [ ComputePipeline ]
//=============================================================================
gl4::ComputePipelineId gl4::CreateComputePipeline(const ComputePipelineInfo& createInfo)
{
	gl4::ComputePipelineId id;
	id.program = CreateShaderProgram(createInfo.shader.data());
	if (!id.program) return {};

	if (!createInfo.debugName.empty())
	{
		glObjectLabel(GL_PROGRAM, id.program, static_cast<GLsizei>(createInfo.debugName.length()), createInfo.debugName.data());
	}

	GLint workgroupSize[3];
	glGetProgramiv(id.program, GL_COMPUTE_WORK_GROUP_SIZE, workgroupSize);

	assert(
		workgroupSize[0] <= gDeviceProperties.limits.maxComputeWorkGroupSize[0] &&
		workgroupSize[1] <= gDeviceProperties.limits.maxComputeWorkGroupSize[1] &&
		workgroupSize[2] <= gDeviceProperties.limits.maxComputeWorkGroupSize[2]);
	assert(workgroupSize[0] * workgroupSize[1] * workgroupSize[2] <=
		gDeviceProperties.limits.maxComputeWorkGroupInvocations);

	id.uniformBlocks = reflectProgram(id.program, GL_UNIFORM_BLOCK);
	id.storageBlocks = reflectProgram(id.program, GL_SHADER_STORAGE_BLOCK);
	id.samplersAndImages = reflectProgram(id.program, GL_UNIFORM);

	id.workgroupSize.width = static_cast<uint32_t>(workgroupSize[0]);
	id.workgroupSize.height = static_cast<uint32_t>(workgroupSize[1]);
	id.workgroupSize.depth = static_cast<uint32_t>(workgroupSize[2]);

	id.valid = true;
	return id;
}
//=============================================================================
#pragma endregion
//=============================================================================
#pragma region [ Cmd ]
//=============================================================================
// helper function
inline void GLEnableOrDisable(GLenum state, GLboolean value)
{
	if (value) glEnable(state);
	else glDisable(state);
}
//=============================================================================
void gl4::Cmd::BindGraphicsPipeline(const GraphicsPipelineId& pipeline, bool resetCacheState)
{
	if (!pipeline.valid)
	{
		Error("GraphicsPipelineId not valid");
		return;
	}

	if (resetCacheState) // сброс кешированного стейта
	{
		lastGraphicsPipeline.valid = false;
		lastProgram = {};
		currentVao = {};
	}

	if (lastProgram != pipeline.program)
	{
		gl4::Bind(pipeline.program);
		lastProgram = pipeline.program;
	}

	if (currentVao != pipeline.vao)
	{
		gl4::Bind(pipeline.vao);
		currentVao = pipeline.vao;
	}

	// стейт уже стоит
	if (lastGraphicsPipeline == pipeline)
	{
#if defined(_DEBUG)
		// TODO: сделать проверку на то что реальный рендер стейт на GAPI именно такой - делать только в отладке, так как надо делать запросы к GAPI
#endif
		return;
	}

	if (isPipelineDebugGroupPushed)
	{
		isPipelineDebugGroupPushed = false;
		glPopDebugGroup();
	}

	if (!pipeline.debugName.empty())
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION,
			0,
			static_cast<GLsizei>(pipeline.debugName.size()),
			pipeline.debugName.data());
		isPipelineDebugGroupPushed = true;
	}

	// всегда включать
	glEnable(GL_FRAMEBUFFER_SRGB);

	//-------------------------------------------------------------------------
	// input assembly
	//-------------------------------------------------------------------------
	const auto& ias = pipeline.inputAssemblyState;
	if (ias.primitiveRestartEnable != lastGraphicsPipeline.inputAssemblyState.primitiveRestartEnable)
	{
		GLEnableOrDisable(GL_PRIMITIVE_RESTART_FIXED_INDEX, ias.primitiveRestartEnable);
	}
	currentTopology = ias.topology;

	//-------------------------------------------------------------------------
	// tessellation
	//-------------------------------------------------------------------------
	const auto& ts = pipeline.tessellationState;
	if (ts.patchControlPoints > 0)
	{
		if (ts.patchControlPoints != lastGraphicsPipeline.tessellationState.patchControlPoints)
		{
			glPatchParameteri(GL_PATCH_VERTICES, static_cast<GLint>(pipeline.tessellationState.patchControlPoints));
		}
	}

	//-------------------------------------------------------------------------
	// rasterization
	//-------------------------------------------------------------------------
	const auto& rs = pipeline.rasterizationState;
	if (rs.depthClampEnable != lastGraphicsPipeline.rasterizationState.depthClampEnable)
	{
		GLEnableOrDisable(GL_DEPTH_CLAMP, rs.depthClampEnable);
	}

	if (rs.polygonMode != lastGraphicsPipeline.rasterizationState.polygonMode)
	{
		glPolygonMode(GL_FRONT_AND_BACK, EnumToGL(rs.polygonMode));
	}

	if (rs.cullMode != lastGraphicsPipeline.rasterizationState.cullMode)
	{
		GLEnableOrDisable(GL_CULL_FACE, rs.cullMode != CullMode::None);
		if (rs.cullMode != CullMode::None)
		{
			glCullFace(EnumToGL(rs.cullMode));
		}
	}

	if (rs.frontFace != lastGraphicsPipeline.rasterizationState.frontFace)
	{
		glFrontFace(EnumToGL(rs.frontFace));
	}

	if (rs.depthBiasEnable != lastGraphicsPipeline.rasterizationState.depthBiasEnable)
	{
		GLEnableOrDisable(GL_POLYGON_OFFSET_FILL, rs.depthBiasEnable);
		GLEnableOrDisable(GL_POLYGON_OFFSET_LINE, rs.depthBiasEnable);
		GLEnableOrDisable(GL_POLYGON_OFFSET_POINT, rs.depthBiasEnable);
	}

	if (rs.depthBiasSlopeFactor != lastGraphicsPipeline.rasterizationState.depthBiasSlopeFactor ||
		rs.depthBiasConstantFactor != lastGraphicsPipeline.rasterizationState.depthBiasConstantFactor)
	{
		glPolygonOffset(rs.depthBiasSlopeFactor, rs.depthBiasConstantFactor);
	}

	if (rs.lineWidth != lastGraphicsPipeline.rasterizationState.lineWidth)
	{
		glLineWidth(rs.lineWidth);
	}

	if (rs.pointSize != lastGraphicsPipeline.rasterizationState.pointSize)
	{
		glPointSize(rs.pointSize);
	}

	//-------------------------------------------------------------------------
	// multisample
	//-------------------------------------------------------------------------
	const auto& ms = pipeline.multisampleState;
	if (ms.sampleShadingEnable != lastGraphicsPipeline.multisampleState.sampleShadingEnable)
	{
		GLEnableOrDisable(GL_SAMPLE_SHADING, ms.sampleShadingEnable);
	}

	if (ms.minSampleShading != lastGraphicsPipeline.multisampleState.minSampleShading)
	{
		glMinSampleShading(ms.minSampleShading);
	}

	if (ms.sampleMask != lastGraphicsPipeline.multisampleState.sampleMask)
	{
		GLEnableOrDisable(GL_SAMPLE_MASK, ms.sampleMask != 0xFFFFFFFF);
		glSampleMaski(0, ms.sampleMask);
	}

	if (ms.alphaToCoverageEnable != lastGraphicsPipeline.multisampleState.alphaToCoverageEnable)
	{
		GLEnableOrDisable(GL_SAMPLE_ALPHA_TO_COVERAGE, ms.alphaToCoverageEnable);
	}

	if (ms.alphaToOneEnable != lastGraphicsPipeline.multisampleState.alphaToOneEnable)
	{
		GLEnableOrDisable(GL_SAMPLE_ALPHA_TO_ONE, ms.alphaToOneEnable);
	}

	//-------------------------------------------------------------------------
	// depth + stencil
	//-------------------------------------------------------------------------
	const auto& ds = pipeline.depthState;
	if (ds.depthTestEnable != lastGraphicsPipeline.depthState.depthTestEnable)
	{
		GLEnableOrDisable(GL_DEPTH_TEST, ds.depthTestEnable);
	}

	if (ds.depthWriteEnable != lastGraphicsPipeline.depthState.depthWriteEnable)
	{
		if (ds.depthWriteEnable != lastDepthMask)
		{
			glDepthMask(ds.depthWriteEnable);
			lastDepthMask = ds.depthWriteEnable;
		}
	}

	if (ds.depthCompareOp != lastGraphicsPipeline.depthState.depthCompareOp)
	{
		glDepthFunc(EnumToGL(ds.depthCompareOp));
	}

	const auto& ss = pipeline.stencilState;
	if (ss.stencilTestEnable != lastGraphicsPipeline.stencilState.stencilTestEnable)
	{
		GLEnableOrDisable(GL_STENCIL_TEST, ss.stencilTestEnable);
	}

	// Stencil front
	if (!lastGraphicsPipeline.stencilState.stencilTestEnable ||
		ss.front != lastGraphicsPipeline.stencilState.front)
	{
		glStencilOpSeparate(GL_FRONT,
			EnumToGL(ss.front.failOp),
			EnumToGL(ss.front.depthFailOp),
			EnumToGL(ss.front.passOp));
		glStencilFuncSeparate(GL_FRONT, EnumToGL(ss.front.compareOp), ss.front.reference, ss.front.compareMask);
		if (lastStencilMask[0] != ss.front.writeMask)
		{
			glStencilMaskSeparate(GL_FRONT, ss.front.writeMask);
			lastStencilMask[0] = ss.front.writeMask;
		}
	}

	// Stencil back
	if (!lastGraphicsPipeline.stencilState.stencilTestEnable ||
		ss.back != lastGraphicsPipeline.stencilState.back)
	{
		glStencilOpSeparate(GL_BACK,
			EnumToGL(ss.back.failOp),
			EnumToGL(ss.back.depthFailOp),
			EnumToGL(ss.back.passOp));
		glStencilFuncSeparate(GL_BACK, EnumToGL(ss.back.compareOp), ss.back.reference, ss.back.compareMask);
		if (lastStencilMask[1] != ss.back.writeMask)
		{
			glStencilMaskSeparate(GL_BACK, ss.back.writeMask);
			lastStencilMask[1] = ss.back.writeMask;
		}
	}

	//-------------------------------------------------------------------------
	// color blending state
	//-------------------------------------------------------------------------
	const auto& cb = pipeline.colorBlendState;
	if (cb.logicOpEnable != lastGraphicsPipeline.colorBlendState.logicOpEnable)
	{
		GLEnableOrDisable(GL_COLOR_LOGIC_OP, cb.logicOpEnable);
		if (!lastGraphicsPipeline.colorBlendState.logicOpEnable ||
			(cb.logicOpEnable && cb.logicOp != lastGraphicsPipeline.colorBlendState.logicOp))
		{
			glLogicOp(EnumToGL(cb.logicOp));
		}
	}

	if (std::memcmp(cb.blendConstants,
		lastGraphicsPipeline.colorBlendState.blendConstants,
		sizeof(cb.blendConstants)) != 0)
	{
		glBlendColor(cb.blendConstants[0], cb.blendConstants[1], cb.blendConstants[2], cb.blendConstants[3]);
	}

	if (cb.attachments.empty() != lastGraphicsPipeline.colorBlendState.attachments.empty())
	{
		GLEnableOrDisable(GL_BLEND, !cb.attachments.empty());
	}

	for (GLuint i = 0; i < static_cast<GLuint>(cb.attachments.size()); i++)
	{
		const auto& cba = cb.attachments[i];
		if (i < lastGraphicsPipeline.colorBlendState.attachments.size() &&
			cba == lastGraphicsPipeline.colorBlendState.attachments[i])
		{
			continue;
		}

		if (cba.blendEnable)
		{
			glBlendFuncSeparatei(i,
				EnumToGL(cba.srcColorBlendFactor),
				EnumToGL(cba.dstColorBlendFactor),
				EnumToGL(cba.srcAlphaBlendFactor),
				EnumToGL(cba.dstAlphaBlendFactor));
			glBlendEquationSeparatei(i, EnumToGL(cba.colorBlendOp), EnumToGL(cba.alphaBlendOp));
		}
		else
		{
			// "no blending" blend state
			glBlendFuncSeparatei(i, GL_SRC_COLOR, GL_ZERO, GL_SRC_ALPHA, GL_ZERO);
			glBlendEquationSeparatei(i, GL_FUNC_ADD, GL_FUNC_ADD);
		}

		if (lastColorMask[i] != cba.colorWriteMask)
		{
			glColorMaski(i,
				(cba.colorWriteMask & ColorComponentFlag::R_BIT) != ColorComponentFlag::NONE,
				(cba.colorWriteMask & ColorComponentFlag::G_BIT) != ColorComponentFlag::NONE,
				(cba.colorWriteMask & ColorComponentFlag::B_BIT) != ColorComponentFlag::NONE,
				(cba.colorWriteMask & ColorComponentFlag::A_BIT) != ColorComponentFlag::NONE);
			lastColorMask[i] = cba.colorWriteMask;
		}
	}

	lastGraphicsPipeline = pipeline;
}
//=============================================================================


void gl4::Cmd::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	glDrawArraysInstancedBaseInstance(EnumToGL(currentTopology), firstVertex, vertexCount, instanceCount, firstInstance);
}
//=============================================================================
void gl4::Cmd::BindVertexBuffer(uint32_t bindingIndex, const BufferStorageId& buffer, uint64_t offset, uint64_t stride)
{
	glVertexArrayVertexBuffer(currentVao, bindingIndex, buffer, static_cast<GLintptr>(offset), static_cast<GLsizei>(stride));
}
//=============================================================================
void gl4::Cmd::BindIndexBuffer(const BufferStorageId& buffer, IndexType indexType)
{
	isIndexBufferBound = true;
	currentIndexType = indexType;
	glVertexArrayElementBuffer(currentVao, buffer);
}
//=============================================================================
#pragma endregion
//=============================================================================