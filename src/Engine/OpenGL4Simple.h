#pragma once

namespace gl4
{
	//-------------------------------------------------------------------------
	// OpenGL Types
	//-------------------------------------------------------------------------
	template <typename Tag>
	struct GLObjectId final
	{
		operator GLuint() const { return id; }

		GLuint id{ 0 };
	};
	struct __ShaderProgramTag;
	struct __BufferTag;
	struct __VertexArrayTag;
	struct __Texture1DTag;
	struct __Texture2DTag;
	struct __Texture3DTag;
	struct __TextureCubeTag;
	struct __Texture1DArrayTag;
	struct __Texture2DArrayTag;
	struct __TextureCubeArrayTag;
	struct __RenderBufferTag;
	struct __FrameBufferTag;

	using ShaderProgramId = GLObjectId<__ShaderProgramTag>;
	using BufferId = GLObjectId<__BufferTag>;
	using VertexArrayId = GLObjectId<__VertexArrayTag>;
	using Texture1DId = GLObjectId<__Texture1DTag>;
	using Texture2DId = GLObjectId<__Texture2DTag>;
	using Texture3DId = GLObjectId<__Texture3DTag>;
	using TextureCubeId = GLObjectId<__TextureCubeTag>;
	using Texture1DArrayId = GLObjectId<__Texture1DArrayTag>;
	using Texture2DArrayId = GLObjectId<__Texture2DArrayTag>;
	using TextureCubeArrayId = GLObjectId<__TextureCubeArrayTag>;
	using RenderBufferId = GLObjectId<__RenderBufferTag>;
	using FrameBufferId = GLObjectId<__FrameBufferTag>;
	
	template<typename T>
	bool IsValid(const T& res) { return res.id > 0; }

	template<typename T>
	void Create(T& res)
	{
		if constexpr (std::is_same_v<T, ShaderProgramId>)         { res.id = glCreateProgram(); }
		else if constexpr (std::is_same_v<T, BufferId>)           { glCreateBuffers(1, &res.id); }
		else if constexpr (std::is_same_v<T, VertexArrayId>)      { glCreateVertexArrays(1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture1DId>)        { glCreateTextures(GL_TEXTURE_1D, 1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture2DId>)        { glCreateTextures(GL_TEXTURE_2D, 1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture3DId>)        { glCreateTextures(GL_TEXTURE_3D, 1, &res.id); }
		else if constexpr (std::is_same_v<T, TextureCubeId>)      { glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture1DArrayId>)   { glCreateTextures(GL_TEXTURE_1D_ARRAY, 1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture2DArrayId>)   { glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &res.id); }
		else if constexpr (std::is_same_v<T, TextureCubeArrayId>) { glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &res.id); }
		else if constexpr (std::is_same_v<T, RenderBufferId>)     { glCreateRenderbuffers( 1, &res.id); }
		else if constexpr (std::is_same_v<T, FrameBufferId>)      { glCreateFramebuffers(1, &res.id); }
		assert(res.id);
	}

	template<typename T>
	void Destroy(T& res)
	{
		if constexpr (std::is_same_v<T, ShaderProgramId>)         { glDeleteProgram(res.id); }
		else if constexpr (std::is_same_v<T, BufferId>)           { glDeleteBuffers(1, &res.id); }
		else if constexpr (std::is_same_v<T, VertexArrayId>)      { glDeleteVertexArrays(1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture1DId>)        { glDeleteTextures(1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture2DId>)        { glDeleteTextures(1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture3DId>)        { glDeleteTextures(1, &res.id); }
		else if constexpr (std::is_same_v<T, TextureCubeId>)      { glDeleteTextures(1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture1DArrayId>)   { glDeleteTextures(1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture2DArrayId>)   { glDeleteTextures(1, &res.id); }
		else if constexpr (std::is_same_v<T, TextureCubeArrayId>) { glDeleteTextures(1, &res.id); }
		else if constexpr (std::is_same_v<T, RenderBufferId>)     { glDeleteRenderbuffers(1, &res.id); }
		else if constexpr (std::is_same_v<T, FrameBufferId>)      { glDeleteFramebuffers(1, &res.id); }
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
	ShaderProgramId CreateShaderProgram(const std::string& computeSrc);
	ShaderProgramId CreateShaderProgram(const std::string& vertexSrc, const std::string& fragmentSrc);
	ShaderProgramId CreateShaderProgram(const std::string& vertexSrc, const std::string& geometrySrc, const std::string& fragmentSrc);

	int GetUniformLocation(ShaderProgramId program, const std::string& name);      // TODO: а нужна ли? это просто glGetUniformLocation
	GLuint GetUniformBlockIndex(ShaderProgramId program, const std::string& name); // TODO: а нужна ли? это просто glGetUniformBlockIndex

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
	void SetUniform(ShaderProgramId program, const std::string& name, bool value);
	void SetUniform(ShaderProgramId program, const std::string& name, int value);
	void SetUniform(ShaderProgramId program, const std::string& name, uint32_t value);
	void SetUniform(ShaderProgramId program, const std::string& name, float value);
	void SetUniform(ShaderProgramId program, const std::string& name, const glm::vec2& value);
	void SetUniform(ShaderProgramId program, const std::string& name, const glm::ivec2& value);
	void SetUniform(ShaderProgramId program, const std::string& name, float x, float y);
	void SetUniform(ShaderProgramId program, const std::string& name, const glm::vec3& value);
	void SetUniform(ShaderProgramId program, const std::string& name, float x, float y, float z);
	void SetUniform(ShaderProgramId program, const std::string& name, const glm::vec4& value);
	void SetUniform(ShaderProgramId program, const std::string& name, float x, float y, float z, float w);
	void SetUniform(ShaderProgramId program, const std::string& name, const glm::mat2& mat);
	void SetUniform(ShaderProgramId program, const std::string& name, const glm::mat3& mat);
	void SetUniform(ShaderProgramId program, const std::string& name, const glm::mat4& mat);

	//-------------------------------------------------------------------------
	// Buffer
	//-------------------------------------------------------------------------

	BufferId CreateBuffer(GLenum usage, GLsizeiptr size, void* data);

	BufferId CreateBufferStorage(GLbitfield flags, GLsizeiptr size, void* data);
	BufferId CreateBufferStorage(GLbitfield flags, GLsizeiptr sizeElement, GLsizeiptr numElement, void* data);

	template<typename T>
	BufferId CreateBufferStorage(GLbitfield flags, const std::vector<T>& data)
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

	VertexArrayId CreateVertexArray();
	VertexArrayId CreateVertexArray(const std::vector<VertexAttribute>& attributes);
	VertexArrayId CreateVertexArray(BufferId vbo, size_t vertexSize, const std::vector<VertexAttribute>& attributes);
	VertexArrayId CreateVertexArray(BufferId vbo, BufferId ibo, size_t vertexSize, const std::vector<VertexAttribute>& attributes);

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

	Texture2DId CreateTexture2D(GLenum internalFormat, GLsizei width, GLsizei height, void* data, const TextureParameter& param = defaultTextureParameter2D);
	Texture2DId LoadTexture2D(const char* texturePath, bool flipVertical = false, const TextureParameter& param = defaultTextureParameter2D);
	Texture2DId LoadTexture2DHDR(const char* texturePath, bool flipVertical = false, const TextureParameter& param = defaultTextureParameter2DHDR);
	TextureCubeId LoadCubeMap(const std::vector<std::string>& files, const std::string& directory);

	void BindTextureSampler(GLuint unit, Texture2DId texture, GLuint sampler);


	//-------------------------------------------------------------------------
	// Framebuffer
	//-------------------------------------------------------------------------
	GLuint CreateColorBuffer2D(int width, int height, GLenum formatColor); // удалить - через создание текстуры
	GLuint CreateDepthBuffer2D(int width, int height, GLenum formatDepth = GL_DEPTH_COMPONENT32);
	// TODO: CreateRenderBuffer???
	FrameBufferId CreateFrameBuffer2D(GLuint colorBuffer, GLuint depthBuffer);


	//-------------------------------------------------------------------------
	// Commands
	//-------------------------------------------------------------------------

	void SetFrameBuffer(gl4::FrameBufferId fbo, int width, int height, GLbitfield clearMask);

} // namespace gl4