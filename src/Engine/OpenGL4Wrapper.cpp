#include "stdafx.h"
#include "OpenGL4Wrapper.h"
#include "Log.h"
//=============================================================================
namespace
{
	GLuint currentFBO{ 0 };
}
//=============================================================================
void ClearOpenGLState()
{
	currentFBO = 0;
}
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
GLuint gl4::CreateShader(GLenum type, const char* shaderSource)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &shaderSource, nullptr);
	glCompileShader(shader);

	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLchar log[512];
		glGetShaderInfoLog(shader, 512, nullptr, log);

		const std::string logError
			= "OPENGL " + shaderTypeToString(type) + ": Shader compilation failed : "
			+ std::string(log) + ", Source: \n" + printShaderSource(shaderSource);
		Error(logError);
	}

	return shader;
}
//=============================================================================
inline void checkProgramStatus(GLuint program)
{
	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		char  log[512];
		glGetProgramInfoLog(program, 512, nullptr, log);
		std::string logError = "OPENGL: Shader program linking failed: " + std::string(log);
		Error(logError);
	}
}
//=============================================================================
GLuint gl4::CreateShaderProgram(const char* computeSrc)
{
	GLuint program = glCreateProgram();
	GLuint shader = CreateShader(GL_COMPUTE_SHADER, computeSrc);
	glAttachShader(program, shader);
	glLinkProgram(program);
	glDeleteShader(shader);
	checkProgramStatus(program);
	return program;
}
//=============================================================================
GLuint gl4::CreateShaderProgram(const char* vertexSrc, const char* fragmentSrc)
{
	return CreateShaderProgram(vertexSrc, nullptr, fragmentSrc);
}
//=============================================================================
GLuint gl4::CreateShaderProgram(const char* vertexSrc, const char* geometrySrc, const char* fragmentSrc)
{
	GLuint program = glCreateProgram();

	std::pair<GLenum, const char*> shaders[] = {
		{GL_VERTEX_SHADER,          vertexSrc},
		{GL_GEOMETRY_SHADER,        geometrySrc},
		{GL_FRAGMENT_SHADER,        fragmentSrc}
	};
	std::vector<GLuint> createdShaders;
	for (const auto& [type, source] : shaders)
	{
		if (source)
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
int gl4::GetUniformLocation(GLuint program, const std::string& name)
{
	return glGetUniformLocation(program, name.c_str());
}
//=============================================================================
GLuint gl4::GetUniformBlockIndex(GLuint program, const std::string& name)
{
	return glGetUniformBlockIndex(program, name.c_str());
}
//=============================================================================
void gl4::SetUniform(int uniformLoc, bool value)
{
	glUniform1i(uniformLoc, static_cast<int>(value));
}
//=============================================================================
void gl4::SetUniform(int uniformLoc, int value)
{
	glUniform1i(uniformLoc, value);
}
//=============================================================================
void gl4::SetUniform(int uniformLoc, float value)
{
	glUniform1f(uniformLoc, value);
}
//=============================================================================
void gl4::SetUniform(int uniformLoc, const glm::vec2& value)
{
	glUniform2fv(uniformLoc, 1, &value[0]);
}
//=============================================================================
void gl4::SetUniform(int uniformLoc, float x, float y)
{
	glUniform2f(uniformLoc, x, y);
}
//=============================================================================
void gl4::SetUniform(int uniformLoc, const glm::vec3& value)
{
	glUniform3fv(uniformLoc, 1, &value[0]);
}
//=============================================================================
void gl4::SetUniform(int uniformLoc, float x, float y, float z)
{
	glUniform3f(uniformLoc, x, y, z);
}
//=============================================================================
void gl4::SetUniform(int uniformLoc, const glm::vec4& value)
{
	glUniform4fv(uniformLoc, 1, &value[0]);
}
//=============================================================================
void gl4::SetUniform(int uniformLoc, float x, float y, float z, float w)
{
	glUniform4f(uniformLoc, x, y, z, w);
}
//=============================================================================
void gl4::SetUniform(int uniformLoc, const glm::mat2& mat)
{
	glUniformMatrix2fv(uniformLoc, 1, GL_FALSE, &mat[0][0]);
}
//=============================================================================
void gl4::SetUniform(int uniformLoc, const glm::mat3& mat)
{
	glUniformMatrix3fv(uniformLoc, 1, GL_FALSE, &mat[0][0]);
}
//=============================================================================
void gl4::SetUniform(int uniformLoc, const glm::mat4& mat)
{
	glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &mat[0][0]);
}
//=============================================================================
void gl4::SetUniform(GLuint program, const std::string& name, bool value)
{
	SetUniform(GetUniformLocation(program, name.c_str()), value);
}
//=============================================================================
void gl4::SetUniform(GLuint program, const std::string& name, int value)
{
	SetUniform(GetUniformLocation(program, name.c_str()), value);
}
//=============================================================================
void gl4::SetUniform(GLuint program, const std::string& name, float value)
{
	SetUniform(GetUniformLocation(program, name.c_str()), value);
}
//=============================================================================
void gl4::SetUniform(GLuint program, const std::string& name, const glm::vec2& value)
{
	SetUniform(GetUniformLocation(program, name.c_str()), value);
}
//=============================================================================
void gl4::SetUniform(GLuint program, const std::string& name, float x, float y)
{
	SetUniform(GetUniformLocation(program, name.c_str()), x, y);
}
//=============================================================================
void gl4::SetUniform(GLuint program, const std::string& name, const glm::vec3& value)
{
	SetUniform(GetUniformLocation(program, name.c_str()), value);
}
//=============================================================================
void gl4::SetUniform(GLuint program, const std::string& name, float x, float y, float z)
{
	SetUniform(GetUniformLocation(program, name.c_str()), x, y, z);
}
//=============================================================================
void gl4::SetUniform(GLuint program, const std::string& name, const glm::vec4& value)
{
	SetUniform(GetUniformLocation(program, name.c_str()), value);
}
//=============================================================================
void gl4::SetUniform(GLuint program, const std::string& name, float x, float y, float z, float w)
{
	SetUniform(GetUniformLocation(program, name.c_str()), x, y, z, w);
}
//=============================================================================
void gl4::SetUniform(GLuint program, const std::string& name, const glm::mat2& mat)
{
	SetUniform(GetUniformLocation(program, name.c_str()), mat);
}
//=============================================================================
void gl4::SetUniform(GLuint program, const std::string& name, const glm::mat3& mat)
{
	SetUniform(GetUniformLocation(program, name.c_str()), mat);
}
//=============================================================================
void gl4::SetUniform(GLuint program, const std::string& name, const glm::mat4& mat)
{
	SetUniform(GetUniformLocation(program, name.c_str()), mat);
}
//=============================================================================
GLuint gl4::CreateBuffer(GLenum usage, GLsizeiptr size, void* data)
{
	GLuint buffer;
	glCreateBuffers(1, &buffer);
	glNamedBufferData(buffer, size, data, usage);
	return buffer;
}
//=============================================================================
GLuint gl4::CreateBufferStorage(GLbitfield flags, GLsizeiptr size, void* data)
{
	GLuint buffer;
	glCreateBuffers(1, &buffer);
	glNamedBufferStorage(buffer, size, data, flags);
	return buffer;
}
//=============================================================================
GLuint gl4::CreateBufferStorage(GLbitfield flags, GLsizeiptr sizeElement, GLsizeiptr numElement, void* data)
{
	return CreateBufferStorage(flags, sizeElement * numElement, data);
}
//=============================================================================
void gl4::SetVertexAttrib(GLuint vao, GLuint attribIndex, GLint size, GLenum type, GLboolean normalized, GLuint relativeOffset)
{
	glEnableVertexArrayAttrib(vao, attribIndex);
	glVertexArrayAttribBinding(vao, attribIndex, 0);

	if (type == GL_INT) // TODO: другие типы возможно тоже учесть, и есть еще glVertexArrayAttribLFormat
		glVertexArrayAttribIFormat(vao, attribIndex, size, type, relativeOffset);
	else
		glVertexArrayAttribFormat(vao, attribIndex, size, type, normalized, relativeOffset);
}
//=============================================================================
void gl4::SetVertexAttrib(GLuint vao, const VertexAttribute& attr)
{
	SetVertexAttrib(vao, attr.index, attr.size, attr.type, attr.normalized ? GL_TRUE : GL_FALSE, attr.relativeOffset);
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
GLuint gl4::CreateVertexArray()
{
	GLuint vao;
	glCreateVertexArrays(1, &vao);
	return vao;
}
//=============================================================================
GLuint gl4::CreateVertexArray(const std::vector<VertexAttribute>& attributes)
{
	GLuint vao = CreateVertexArray();
	SetVertexAttrib(vao, attributes);
	return vao;
}
//=============================================================================
GLuint gl4::CreateVertexArray(GLuint vbo, size_t vertexSize, const std::vector<VertexAttribute>& attributes)
{
	return CreateVertexArray(vbo, 0, vertexSize, attributes);
}
//=============================================================================
GLuint gl4::CreateVertexArray(GLuint vbo, GLuint ibo, size_t vertexSize, const std::vector<VertexAttribute>& attributes)
{
	GLuint vao = CreateVertexArray(attributes);
	if (vbo > 0) glVertexArrayVertexBuffer(vao, 0, vbo, 0, vertexSize);
	if (ibo > 0) glVertexArrayElementBuffer(vao, ibo);
	return vao;
}
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
GLuint gl4::CreateTexture2D(GLenum internalFormat, GLsizei width, GLsizei height, void* data, const TextureParameter& param)
{
	const int numMipmaps = param.genMipMap ? getNumMipMapLevels2D(width, height) : 1;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	GLuint texture;
	glCreateTextures(GL_TEXTURE_2D, 1, &texture);

	glTextureParameteri(texture, GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, param.minFilter);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, param.magFilter);
	glTextureParameteri(texture, GL_TEXTURE_MAX_ANISOTROPY, param.maxAnisotropy);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_S, param.wrap);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_T, param.wrap);

	glTextureStorage2D(texture, numMipmaps, internalFormat, width, height);
	if (data)
	{
		glTextureSubImage2D(texture, 0, 0, 0, width, height, getBaseFormat(internalFormat), param.dataType, data);
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
GLuint gl4::LoadTexture2D(const char* texturePath, bool flipVertical, const TextureParameter& param)
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
			return 0;
		}
	}

	GLenum internalFormat = GL_RGBA8;
	if (nrChannels == 1)      internalFormat = GL_R8;
	else if (nrChannels == 2) internalFormat = GL_RG8;
	else if (nrChannels == 3) internalFormat = GL_RGB8;

	GLuint texture = CreateTexture2D(internalFormat, width, height, data, param);

	stbi_image_free(data);
	return texture;
}
//=============================================================================
GLuint gl4::LoadTexture2DHDR(const char* texturePath, bool flipVertical, const TextureParameter& param)
{
	// TODO: возможно объединить с LoadTexture2D

	stbi_set_flip_vertically_on_load(flipVertical);

	int width, height, nrChannels;
	float* data = stbi_loadf(texturePath, &width, &height, &nrChannels, 0);
	if (!data)
	{
		Error((std::string("Texture: ") + texturePath + " not find").c_str());
		// TODO: создание дефолтной текстуры
		return 0;
	}
	GLuint texture = CreateTexture2D(GL_RGB32F, width, height, data, param);
	stbi_image_free(data);
	return texture;
}
//=============================================================================
GLuint gl4::LoadCubeMap(const std::vector<std::string>& files, const std::string& directory)
{
	// TODO: возможность настроить через TextureParameter

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	GLuint texture;
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &texture);

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
			return 0;
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
void gl4::BindTextureSampler(GLuint unit, GLuint texture, GLuint sampler)
{
	glBindTextureUnit(unit, texture);
	glBindSampler(unit, sampler);
}
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
GLuint gl4::CreateFrameBuffer2D(GLuint colorBuffer, GLuint depthBuffer)
{
	GLuint framebuffer;
	glCreateFramebuffers(1, &framebuffer);

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
void gl4::SetFrameBuffer(GLuint fbo, int width, int height, GLbitfield clearMask)
{
	if (currentFBO != fbo)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		currentFBO = fbo;
	}

	glViewport(0, 0, width, height);
	glClear(clearMask);
}
//=============================================================================