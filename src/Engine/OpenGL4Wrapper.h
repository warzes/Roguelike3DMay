#pragma once

namespace gl4
{
	//-------------------------------------------------------------------------
	// Shader
	//-------------------------------------------------------------------------

	GLuint CreateShader(GLenum type, const char* shaderSource);
	GLuint CreateShaderProgram(const char* computeSrc);
	GLuint CreateShaderProgram(const char* vertexSrc, const char* fragmentSrc);
	GLuint CreateShaderProgram(const char* vertexSrc, const char* geometrySrc, const char* fragmentSrc);

	int GetUniformLocation(GLuint program, const std::string& name);      // TODO: а нужна ли? это просто glGetUniformLocation
	GLuint GetUniformBlockIndex(GLuint program, const std::string& name); // TODO: а нужна ли? это просто glGetUniformBlockIndex

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

	// Временные для демок, использовать нежелательно
	void SetUniform(GLuint program, const std::string& name, bool value);
	void SetUniform(GLuint program, const std::string& name, int value);
	void SetUniform(GLuint program, const std::string& name, float value);
	void SetUniform(GLuint program, const std::string& name, const glm::vec2& value);
	void SetUniform(GLuint program, const std::string& name, float x, float y);
	void SetUniform(GLuint program, const std::string& name, const glm::vec3& value);
	void SetUniform(GLuint program, const std::string& name, float x, float y, float z);
	void SetUniform(GLuint program, const std::string& name, const glm::vec4& value);
	void SetUniform(GLuint program, const std::string& name, float x, float y, float z, float w);
	void SetUniform(GLuint program, const std::string& name, const glm::mat2& mat);
	void SetUniform(GLuint program, const std::string& name, const glm::mat3& mat);
	void SetUniform(GLuint program, const std::string& name, const glm::mat4& mat);

	//-------------------------------------------------------------------------
	// Buffer
	//-------------------------------------------------------------------------

	GLuint CreateBuffer(GLbitfield flags, GLsizeiptr size, void* data);

	GLuint CreateBufferStorage(GLbitfield flags, GLsizeiptr size, void* data);
	GLuint CreateBufferStorage(GLbitfield flags, GLsizeiptr sizeElement, GLsizeiptr numElement, void* data);

	template<typename T>
	GLuint CreateBufferStorage(GLbitfield flags, const std::vector<T>& data)
	{
		return CreateBufferStorage(flags, sizeof(T), data.size(), (void*)data.data());
	}

	//-------------------------------------------------------------------------
	// Vertex Array
	//-------------------------------------------------------------------------

	struct VertexAttribute final
	{
		//  TODO: для случая type = GL_INT не нужно передавать normalized. Подумать как сделать
		GLuint index;				// example: 0
		GLint size;					// example: 3
		GLenum type;				// example: GL_FLOAT
		bool normalized{ false };	// example: GL_FALSE
		GLuint relativeOffset;		// example: offsetof(Vertex, pos)
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
		GLint   minFilter{ GL_LINEAR_MIPMAP_LINEAR };
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

	GLuint CreateColorBuffer2D(int width, int height, GLenum formatColor); // удалить - через создание текстуры
	GLuint CreateDepthBuffer2D(int width, int height, GLenum formatDepth = GL_DEPTH_COMPONENT32);
	// TODO: CreateRenderBuffer???
	GLuint CreateFrameBuffer2D(GLuint colorBuffer, GLuint depthBuffer);


	//-------------------------------------------------------------------------
	// Commands
	//-------------------------------------------------------------------------

	void SetFrameBuffer(GLuint fbo, int width, int height, GLbitfield clearMask);

}