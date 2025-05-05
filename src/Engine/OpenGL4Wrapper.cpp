#include "stdafx.h"
#include "OpenGL4Wrapper.h"
#include "Log.h"
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

	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		char  log[512];
		glGetProgramInfoLog(program, 512, nullptr, log);
		std::string logError = "OPENGL: Shader program linking failed: " + std::string(log);
		Error(logError);
	}
	return program;
}
//=============================================================================
GLuint gl4::CreateBuffer(GLbitfield flags, GLsizeiptr size, void* data)
{
	GLuint buffer;
	glCreateBuffers(1, &buffer);
	glNamedBufferStorage(buffer, size, data, flags);
	return buffer;
}
//=============================================================================
void gl4::SetVertexAttrib(GLuint vao, GLuint attribIndex, GLint size, GLenum type, GLboolean normalized, GLuint relativeOffset)
{
	glEnableVertexArrayAttrib(vao, attribIndex);
	glVertexArrayAttribBinding(vao, attribIndex, 0);
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
inline int getNumMipMapLevels2D(int w, int h)
{
	int levels = 1;
	while ((w | h) >> levels)
		levels += 1;
	return levels;
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
GLuint gl4::CreateColorBuffer2D(int width, int height, GLenum formatColor)
{
	TextureParameter param = defaultTextureParameter2D;
	param.wrap = GL_CLAMP_TO_EDGE;
	return gl4::CreateTexture2D(formatColor, width, height, nullptr, param);
}
//=============================================================================
GLuint gl4::CreateDepthBuffer2D(int width, int height, GLenum formatDepth)
{
	TextureParameter param = defaultTextureParameter2D;
	param.wrap = GL_CLAMP_TO_BORDER;
	GLuint texture = gl4::CreateTexture2D(formatDepth, width, height, nullptr, param);
	const GLfloat border[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glTextureParameterfv(texture, GL_TEXTURE_BORDER_COLOR, border);

	return texture;
}
//=============================================================================
GLuint gl4::CreateFrameBuffer2D(int width, int height, GLuint colorBuffer, GLuint depthBuffer)
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