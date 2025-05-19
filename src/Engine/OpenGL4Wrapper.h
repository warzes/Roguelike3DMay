#pragma once

namespace gl4
{
	//-------------------------------------------------------------------------
	// OpenGL Types
	//-------------------------------------------------------------------------
	template <typename Tag>
	struct GLObject final
	{
		operator GLuint() const { return id; }

		GLuint id{ 0 };
	};
	struct ShaderProgramTag {};
	struct BufferTag {};
	struct VertexArrayTag {};
	struct Texture1DTag {};
	struct Texture2DTag {};
	struct Texture3DTag {};
	struct TextureCubeTag {};
	struct Texture1DArrayTag {};
	struct Texture2DArrayTag {};
	struct TextureCubeArrayTag {};
	struct RenderBufferTag {};
	struct FrameBufferTag {};

	using ShaderProgram = GLObject<ShaderProgramTag>;
	using Buffer = GLObject<BufferTag>;
	using VertexArray = GLObject<VertexArrayTag>;
	using Texture1D = GLObject<Texture1DTag>;
	using Texture2D = GLObject<Texture2DTag>;
	using Texture3D = GLObject<Texture3DTag>;
	using TextureCube = GLObject<TextureCubeTag>;
	using Texture1DArray = GLObject<Texture1DArrayTag>;
	using Texture2DArray = GLObject<Texture2DArrayTag>;
	using TextureCubeArray = GLObject<TextureCubeArrayTag>;
	using RenderBuffer = GLObject<RenderBufferTag>;
	using FrameBuffer = GLObject<FrameBufferTag>;
	
	template<typename T>
	bool IsValid(const T& res) { return res.id > 0; }

	template<typename T>
	void Create(T& res)
	{
		if constexpr (std::is_same_v<T, ShaderProgram>)         { res.id = glCreateProgram(); }
		else if constexpr (std::is_same_v<T, Buffer>)           { glCreateBuffers(1, &res.id); }
		else if constexpr (std::is_same_v<T, VertexArray>)      { glCreateVertexArrays(1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture1D>)        { glCreateTextures(GL_TEXTURE_1D, 1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture2D>)        { glCreateTextures(GL_TEXTURE_2D, 1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture3D>)        { glCreateTextures(GL_TEXTURE_3D, 1, &res.id); }
		else if constexpr (std::is_same_v<T, TextureCube>)      { glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture1DArray>)   { glCreateTextures(GL_TEXTURE_1D_ARRAY, 1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture2DArray>)   { glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &res.id); }
		else if constexpr (std::is_same_v<T, TextureCubeArray>) { glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &res.id); }
		else if constexpr (std::is_same_v<T, RenderBuffer>)     { glCreateRenderbuffers( 1, &res.id); }
		else if constexpr (std::is_same_v<T, FrameBuffer>)      { glCreateFramebuffers(1, &res.id); }
		assert(res.id);
	}

	template<typename T>
	void Destroy(T& res)
	{
		if constexpr (std::is_same_v<T, ShaderProgram>)         { glDeleteProgram(res.id); }
		else if constexpr (std::is_same_v<T, Buffer>)           { glDeleteBuffers(1, &res.id); }
		else if constexpr (std::is_same_v<T, VertexArray>)      { glDeleteVertexArrays(1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture1D>)        { glDeleteTextures(1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture2D>)        { glDeleteTextures(1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture3D>)        { glDeleteTextures(1, &res.id); }
		else if constexpr (std::is_same_v<T, TextureCube>)      { glDeleteTextures(1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture1DArray>)   { glDeleteTextures(1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture2DArray>)   { glDeleteTextures(1, &res.id); }
		else if constexpr (std::is_same_v<T, TextureCubeArray>) { glDeleteTextures(1, &res.id); }
		else if constexpr (std::is_same_v<T, RenderBuffer>)     { glDeleteRenderbuffers(1, &res.id); }
		else if constexpr (std::is_same_v<T, FrameBuffer>)      { glDeleteFramebuffers(1, &res.id); }
		res.id = 0;
	}

	/*
	класс использующий принцип raii для объектов. Возможно в будущем пригодится
	- реализовать методы перемещения и запретить копирование
	- реализовать методы каста в базовый тип, чтобы все функции ниже автоматически извлекали нужный тип, без ручного каста
	*/
	template<typename T>
	struct Raii
	{
		Raii() : id(m_type.id)
		{
			gl4::Create(m_type);
		}
		~Raii()
		{
			gl4::Destroy(m_type);
		}

		GLuint& id;
	private:
		T m_type;
	};

	//-------------------------------------------------------------------------
	// Shader
	//-------------------------------------------------------------------------

	GLuint CreateShader(GLenum type, const std::string& shaderSource);
	ShaderProgram CreateShaderProgram(const std::string& computeSrc);
	ShaderProgram CreateShaderProgram(const std::string& vertexSrc, const std::string& fragmentSrc);
	ShaderProgram CreateShaderProgram(const std::string& vertexSrc, const std::string& geometrySrc, const std::string& fragmentSrc);

	int GetUniformLocation(ShaderProgram program, const std::string& name);      // TODO: а нужна ли? это просто glGetUniformLocation
	GLuint GetUniformBlockIndex(ShaderProgram program, const std::string& name); // TODO: а нужна ли? это просто glGetUniformBlockIndex

	void SetUniform(int uniformLoc, bool value);
	void SetUniform(int uniformLoc, int value);
	void SetUniform(int uniformLoc, uint32_t value);
	void SetUniform(int uniformLoc, float value);
	void SetUniform(int uniformLoc, const glm::vec2& value);
	void SetUniform(int uniformLoc, const glm::ivec2& value);
	void SetUniform(int uniformLoc, float x, float y);
	void SetUniform(int uniformLoc, const glm::vec3& value);
	void SetUniform(int uniformLoc, float x, float y, float z);
	void SetUniform(int uniformLoc, const glm::vec4& value);
	void SetUniform(int uniformLoc, float x, float y, float z, float w);
	void SetUniform(int uniformLoc, const glm::mat2& mat);
	void SetUniform(int uniformLoc, const glm::mat3& mat);
	void SetUniform(int uniformLoc, const glm::mat4& mat);

	// Временные для демок, использовать нежелательно
	void SetUniform(ShaderProgram program, const std::string& name, bool value);
	void SetUniform(ShaderProgram program, const std::string& name, int value);
	void SetUniform(ShaderProgram program, const std::string& name, uint32_t value);
	void SetUniform(ShaderProgram program, const std::string& name, float value);
	void SetUniform(ShaderProgram program, const std::string& name, const glm::vec2& value);
	void SetUniform(ShaderProgram program, const std::string& name, const glm::ivec2& value);
	void SetUniform(ShaderProgram program, const std::string& name, float x, float y);
	void SetUniform(ShaderProgram program, const std::string& name, const glm::vec3& value);
	void SetUniform(ShaderProgram program, const std::string& name, float x, float y, float z);
	void SetUniform(ShaderProgram program, const std::string& name, const glm::vec4& value);
	void SetUniform(ShaderProgram program, const std::string& name, float x, float y, float z, float w);
	void SetUniform(ShaderProgram program, const std::string& name, const glm::mat2& mat);
	void SetUniform(ShaderProgram program, const std::string& name, const glm::mat3& mat);
	void SetUniform(ShaderProgram program, const std::string& name, const glm::mat4& mat);

	//-------------------------------------------------------------------------
	// Buffer
	//-------------------------------------------------------------------------

	GLuint CreateBuffer(GLenum usage, GLsizeiptr size, void* data);

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