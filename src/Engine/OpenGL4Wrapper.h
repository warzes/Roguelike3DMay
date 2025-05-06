#pragma once

namespace gl4
{
	//-------------------------------------------------------------------------
	// Shader
	//-------------------------------------------------------------------------

	GLuint CreateShader(GLenum type, const char* shaderSource);
	GLuint CreateShaderProgram(const char* vertexSrc, const char* fragmentSrc);
	GLuint CreateShaderProgram(const char* vertexSrc, const char* geometrySrc, const char* fragmentSrc);

	int GetUniformLocation(GLuint program, const std::string& name);

	void SetUniform(int uniformLoc, bool value);
	void SetUniform(int uniformLoc, int value);
	void SetUniform(int uniformLoc, float value);
	void SetUniform(int uniformLoc, const glm::vec2& value);
	void SetUniform(int uniformLoc, float x, float y);
	void SetUniform(int uniformLoc, const glm::vec3& value);
	void SetUniform(int uniformLoc, float x, float y, float z);
	void SetUniform(int uniformLoc, const glm::vec4& value);
	void SetUniform(int uniformLoc, float x, float y, float z, float w);
	void SetUniform(int uniformLoc, const glm::mat2& mat);
	void SetUniform(int uniformLoc, const glm::mat3& mat);
	void SetUniform(int uniformLoc, const glm::mat4& mat);

	//-------------------------------------------------------------------------
	// Buffer
	//-------------------------------------------------------------------------

	GLuint CreateBuffer(GLbitfield flags, GLsizeiptr size, void* data);

	//-------------------------------------------------------------------------
	// Vertex Array
	//-------------------------------------------------------------------------

	struct VertexAttribute final
	{
		GLuint index;			// example: 0
		GLint size;				// example: 3
		GLenum type;			// example: GL_FLOAT
		bool normalized;		// example: GL_FALSE
		GLuint relativeOffset;	// example: offsetof(Vertex, pos)
	};

	// example:
	//	SetVertexAttrib(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
	//	SetVertexAttrib(vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
	void SetVertexAttrib(GLuint vao, GLuint attribIndex, GLint size, GLenum type, GLboolean normalized, GLuint relativeOffset);
	void SetVertexAttrib(GLuint vao, const VertexAttribute& attribute);
	void SetVertexAttrib(GLuint vao, const std::vector<VertexAttribute>& attributes);

	GLuint CreateVertexArray();
	GLuint CreateVertexArray(const std::vector<VertexAttribute>& attributes);
	GLuint CreateVertexArray(GLuint vbo, size_t vertexSize, const std::vector<VertexAttribute>& attributes);
	GLuint CreateVertexArray(GLuint vbo, GLuint ibo, size_t vertexSize, const std::vector<VertexAttribute>& attributes);

	//-------------------------------------------------------------------------
	// Texture
	//-------------------------------------------------------------------------

	struct TextureParameter final
	{
		GLint   minFilter{ GL_LINEAR };
		GLint   magFilter{ GL_LINEAR };
		uint8_t maxAnisotropy{ 16 };
		GLint   wrap{ GL_REPEAT };

		GLenum  dataType{ GL_UNSIGNED_BYTE };

		bool    genMipMap{ false };
	};
	constexpr TextureParameter defaultTextureParameter2D{};
	constexpr TextureParameter defaultTextureParameter2DHDR{
	.minFilter = GL_LINEAR_MIPMAP_LINEAR,
	.magFilter = GL_LINEAR,
	.maxAnisotropy = 16,
	.wrap = GL_CLAMP_TO_EDGE,
	.dataType = GL_FLOAT,
	.genMipMap = true
	};
	constexpr TextureParameter defaultTextureParameterCube{
		.minFilter = GL_LINEAR_MIPMAP_LINEAR,
		.magFilter = GL_LINEAR,
		.maxAnisotropy = 16,
		.wrap = GL_CLAMP_TO_EDGE,
		.dataType = GL_FLOAT,
		.genMipMap = true
	};

	GLuint CreateTexture2D(GLenum internalFormat, GLsizei width, GLsizei height, void* data, const TextureParameter& param = defaultTextureParameter2D);
	GLuint LoadTexture2D(const char* texturePath, bool flipVertical = false, const TextureParameter& param = defaultTextureParameter2D);
	GLuint LoadTexture2DHDR(const char* texturePath, bool flipVertical = false, const TextureParameter& param = defaultTextureParameter2DHDR);
	GLuint LoadCubeMap(const std::vector<std::string>& files, const std::string& directory);

	void BindTextureSampler(GLuint unit, GLuint texture, GLuint sampler);


	//-------------------------------------------------------------------------
	// Framebuffer
	//-------------------------------------------------------------------------

	GLuint CreateColorBuffer2D(int width, int height, GLenum formatColor);
	GLuint CreateDepthBuffer2D(int width, int height, GLenum formatDepth = GL_DEPTH_COMPONENT32);
	// TODO: CreateRenderBuffer???
	GLuint CreateFrameBuffer2D(int width, int height, GLuint colorBuffer, GLuint depthBuffer);
}