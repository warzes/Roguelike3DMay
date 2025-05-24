#pragma once

#include "OpenGL4ApiToEnum.h"
#include "OpenGL4Shader.h"

// Остается чистой и минимальной оберткой над OpenGL

/*
TODO:
 есть текстурный буфер (создается буфер и текстура, затем буфер грузится в текстуру через glTextureBuffer или glTextureBufferRange. Текстура создается с GL_TEXTURE_BUFFER типом
		GLuint buffer;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_TEXTURE_BUFFER, buffer);
		glBufferData(GL_TEXTURE_BUFFER, sizeInBytes, data, GL_STATIC_DRAW);
		GLuint texBuffer;
		glCreateTextures(GL_TEXTURE_BUFFER, 1, &texBuffer);
		glTextureBuffer(texBuffer, GL_R32F, buffer); // например, один float на пиксель

		uniform samplerBuffer myBuffer;
		void main() {
			float value = texelFetch(myBuffer, index).r;
		}
	но возможно хватит SSBO
*/

namespace gl4
{
	
	//-------------------------------------------------------------------------
	// OpenGL RHI Types
	//-------------------------------------------------------------------------
#pragma region [ OpenGL RHI Types ]

	constexpr inline uint64_t WHOLE_BUFFER = static_cast<uint64_t>(-1);

	template <typename Tag>
	struct GLObjectId
	{
		operator GLuint() const { return id; }

		GLuint id{ 0 };
	};
	struct __ShaderProgramTag;
	struct __BufferTag;
	struct __BufferStorageTag;
	struct __VertexArrayTag;
	struct __TextureTag;
	struct __TextureViewTag;
	struct __Texture1DTag;
	struct __Texture2DTag;
	struct __Texture3DTag;
	struct __TextureCubeTag;
	struct __Texture1DArrayTag;
	struct __Texture2DArrayTag;
	struct __TextureCubeArrayTag;
	struct __SamplerTag;
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
	using SamplerId = GLObjectId<__SamplerTag>;
	using RenderBufferId = GLObjectId<__RenderBufferTag>;
	using FrameBufferId = GLObjectId<__FrameBufferTag>;

	struct BufferStorageId final : public GLObjectId<__BufferStorageTag>
	{
		void* mappedMemory{ nullptr };
		size_t size{ 0 };
		BufferStorageFlags storageFlags{ 0 };
	};

	struct TextureId final : public GLObjectId<__TextureTag>
	{
		TextureCreateInfo info{};
		uint64_t bindlessHandle{ 0 };
	};

	struct TextureViewId final : public GLObjectId<__TextureViewTag>
	{
		TextureViewCreateInfo info{};
	};

	template<typename T>
	bool IsValid(T res) { return res.id > 0; }

	template<typename T>
	void Destroy(T& res)
	{
		if (res.id == 0) return;

		if constexpr (std::is_same_v<T, ShaderProgramId>) { glDeleteProgram(res.id); }
		else if constexpr (std::is_same_v<T, BufferId>) { glDeleteBuffers(1, &res.id); }
		else if constexpr (std::is_same_v<T, BufferStorageId>) 
		{ 
			if (res.mappedMemory)
			{
				glUnmapNamedBuffer(res.id);
				res.mappedMemory = nullptr;
			}
			glDeleteBuffers(1, &res.id); 
		}
		else if constexpr (std::is_same_v<T, VertexArrayId>) { glDeleteVertexArrays(1, &res.id); }
		else if constexpr (std::is_same_v<T, TextureId>)
		{
			if (res.bindlessHandle != 0)
				glMakeTextureHandleNonResidentARB(res.bindlessHandle);
			res.bindlessHandle = 0;
			glDeleteTextures(1, &res.id);
		}
		else if constexpr (std::is_same_v<T, TextureViewId>) { glDeleteTextures(1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture1DId>) { glDeleteTextures(1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture2DId>) { glDeleteTextures(1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture3DId>) { glDeleteTextures(1, &res.id); }
		else if constexpr (std::is_same_v<T, TextureCubeId>) { glDeleteTextures(1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture1DArrayId>) { glDeleteTextures(1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture2DArrayId>) { glDeleteTextures(1, &res.id); }
		else if constexpr (std::is_same_v<T, TextureCubeArrayId>) { glDeleteTextures(1, &res.id); }
		else if constexpr (std::is_same_v<T, SamplerId>) { glDeleteSamplers(1, &res.id); }
		else if constexpr (std::is_same_v<T, RenderBufferId>) { glDeleteRenderbuffers(1, &res.id); }
		else if constexpr (std::is_same_v<T, FrameBufferId>) { glDeleteFramebuffers(1, &res.id); }
		res.id = 0;
	}

#pragma endregion
		
	//-------------------------------------------------------------------------
	// Shader
	//-------------------------------------------------------------------------
#pragma region [ Shader ]

	GLuint CreateShader(GLenum type, const std::string& sourceCode, std::string_view name = "");
	GLuint CreateShaderSpirv(GLenum type, const ShaderSpirvInfo& spirvInfo, std::string_view name = "");

	std::string GetShaderSourceCode(GLuint id);


#pragma endregion

	//-------------------------------------------------------------------------
	// ShaderProgram
	//-------------------------------------------------------------------------
#pragma region [ ShaderProgram ]

	ShaderProgramId CreateShaderProgram(const std::string& computeSrc);
	ShaderProgramId CreateShaderProgram(const std::string& vertexSrc, const std::string& fragmentSrc);
	ShaderProgramId CreateShaderProgram(const std::string& vertexSrc, const std::string& geometrySrc, const std::string& fragmentSrc);

	void Bind(ShaderProgramId id);

	int GetUniformLocation(ShaderProgramId program, const std::string& name);      // TODO: а нужна ли? это просто glGetUniformLocation
	GLuint GetUniformBlockIndex(ShaderProgramId program, const std::string& name); // TODO: а нужна ли? это просто glGetUniformBlockIndex

#pragma region [ SetUniform ]

	void SetUniform(ShaderProgramId program, int uniformLoc, bool value);
	void SetUniform(ShaderProgramId program, int uniformLoc, int value);
	void SetUniform(ShaderProgramId program, int uniformLoc, int v1, int v2);
	void SetUniform(ShaderProgramId program, int uniformLoc, int v1, int v2, int v3);
	void SetUniform(ShaderProgramId program, int uniformLoc, int v1, int v2, int v3, int v4);
	void SetUniform(ShaderProgramId program, int uniformLoc, uint32_t value);
	void SetUniform(ShaderProgramId program, int uniformLoc, uint32_t v1, uint32_t v2);
	void SetUniform(ShaderProgramId program, int uniformLoc, uint32_t v1, uint32_t v2, uint32_t v3);
	void SetUniform(ShaderProgramId program, int uniformLoc, uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4);
	void SetUniform(ShaderProgramId program, int uniformLoc, float value);
	void SetUniform(ShaderProgramId program, int uniformLoc, float v1, float v2);
	void SetUniform(ShaderProgramId program, int uniformLoc, float v1, float v2, float v3);
	void SetUniform(ShaderProgramId program, int uniformLoc, float v1, float v2, float v3, float v4);
	void SetUniform(ShaderProgramId program, int uniformLoc, const glm::vec2& value);
	void SetUniform(ShaderProgramId program, int uniformLoc, const glm::ivec2& value);
	void SetUniform(ShaderProgramId program, int uniformLoc, const glm::uvec2& value);
	void SetUniform(ShaderProgramId program, int uniformLoc, const glm::vec3& value);
	void SetUniform(ShaderProgramId program, int uniformLoc, const glm::ivec3& value);
	void SetUniform(ShaderProgramId program, int uniformLoc, const glm::uvec3& value);
	void SetUniform(ShaderProgramId program, int uniformLoc, const glm::vec4& value);
	void SetUniform(ShaderProgramId program, int uniformLoc, const glm::ivec4& value);
	void SetUniform(ShaderProgramId program, int uniformLoc, const glm::uvec4& value);
	void SetUniform(ShaderProgramId program, int uniformLoc, const glm::mat2& mat, bool transpose = false);
	void SetUniform(ShaderProgramId program, int uniformLoc, const glm::mat3& mat, bool transpose = false);
	void SetUniform(ShaderProgramId program, int uniformLoc, const glm::mat4& mat, bool transpose = false);
	void SetUniform(ShaderProgramId program, int uniformLoc, const glm::mat2x3& mat, bool transpose = false);
	void SetUniform(ShaderProgramId program, int uniformLoc, const glm::mat3x2& mat, bool transpose = false);
	void SetUniform(ShaderProgramId program, int uniformLoc, const glm::mat2x4& mat, bool transpose = false);
	void SetUniform(ShaderProgramId program, int uniformLoc, const glm::mat4x2& mat, bool transpose = false);
	void SetUniform(ShaderProgramId program, int uniformLoc, const glm::mat3x4& mat, bool transpose = false);
	void SetUniform(ShaderProgramId program, int uniformLoc, const glm::mat4x3& mat, bool transpose = false);

	void SetUniform(ShaderProgramId program, const std::string& locName, bool value);
	void SetUniform(ShaderProgramId program, const std::string& locName, int value);
	void SetUniform(ShaderProgramId program, const std::string& locName, int v1, int v2);
	void SetUniform(ShaderProgramId program, const std::string& locName, int v1, int v2, int v3);
	void SetUniform(ShaderProgramId program, const std::string& locName, int v1, int v2, int v3, int v4);
	void SetUniform(ShaderProgramId program, const std::string& locName, uint32_t value);
	void SetUniform(ShaderProgramId program, const std::string& locName, uint32_t v1, uint32_t v2);
	void SetUniform(ShaderProgramId program, const std::string& locName, uint32_t v1, uint32_t v2, uint32_t v3);
	void SetUniform(ShaderProgramId program, const std::string& locName, uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4);
	void SetUniform(ShaderProgramId program, const std::string& locName, float value);
	void SetUniform(ShaderProgramId program, const std::string& locName, float v1, float v2);
	void SetUniform(ShaderProgramId program, const std::string& locName, float v1, float v2, float v3);
	void SetUniform(ShaderProgramId program, const std::string& locName, float v1, float v2, float v3, float v4);
	void SetUniform(ShaderProgramId program, const std::string& locName, const glm::vec2& value);
	void SetUniform(ShaderProgramId program, const std::string& locName, const glm::ivec2& value);
	void SetUniform(ShaderProgramId program, const std::string& locName, const glm::uvec2& value);
	void SetUniform(ShaderProgramId program, const std::string& locName, const glm::vec3& value);
	void SetUniform(ShaderProgramId program, const std::string& locName, const glm::ivec3& value);
	void SetUniform(ShaderProgramId program, const std::string& locName, const glm::uvec3& value);
	void SetUniform(ShaderProgramId program, const std::string& locName, const glm::vec4& value);
	void SetUniform(ShaderProgramId program, const std::string& locName, const glm::ivec4& value);
	void SetUniform(ShaderProgramId program, const std::string& locName, const glm::uvec4& value);
	void SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat2& mat, bool transpose = false);
	void SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat3& mat, bool transpose = false);
	void SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat4& mat, bool transpose = false);
	void SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat2x3& mat, bool transpose = false);
	void SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat3x2& mat, bool transpose = false);
	void SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat2x4& mat, bool transpose = false);
	void SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat4x2& mat, bool transpose = false);
	void SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat3x4& mat, bool transpose = false);
	void SetUniform(ShaderProgramId program, const std::string& locName, const glm::mat4x3& mat, bool transpose = false);

#pragma endregion

#pragma endregion

	//-------------------------------------------------------------------------
	// ProgramPipeline
	//-------------------------------------------------------------------------
	
	// TODO:
	/*
		glCreateProgramPipelines(1, &id);
		glDeleteProgramPipelines(1, &id);

		glUseProgramStages(id, GLbitfield(stages), program);

		glActiveShaderProgram(id, program);

		glValidateProgramPipeline(id);
	*/

	//-------------------------------------------------------------------------
	// Buffer (OLD)
	//-------------------------------------------------------------------------
#pragma region [ Buffer ]

	BufferId CreateBuffer(GLenum usage, GLsizeiptr size, const void* data);

	BufferId CreateBufferStorage(GLbitfield flags, GLsizeiptr size, const void* data);
	BufferId CreateBufferStorage(GLbitfield flags, GLsizeiptr sizeElement, GLsizeiptr numElement, const void* data);
	template<typename T>
	BufferId CreateBufferStorage(GLbitfield flags, const std::vector<T>& data)
	{
		return CreateBufferStorage(flags, sizeof(T), data.size(), data.data());
	}	

	// TODO: Set(Init)Data и Set(Init)Storage???
	void SetSubData(BufferId id, GLintptr offset, GLsizeiptr size, const void* data);
	void CopySubData(BufferId readBuffer, BufferId writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);

	void ClearData(BufferId id, GLenum internalFormat, GLenum format, GLenum type, const void* data);
	void ClearSubData(BufferId id, GLenum internalFormat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data);

	void InvalidateData(BufferId id);
	void InvalidateSubData(BufferId id, GLintptr offset, GLsizeiptr length);

	void* Map(BufferId id, GLenum access);
	void* MapRange(BufferId id, GLintptr offset, GLsizeiptr length, GLbitfield access);
	bool UnMap(BufferId id);
	void FlushMappedRange(BufferId id, GLintptr offset, GLsizeiptr length);

	void* GetBufferPointer(BufferId id);
	void GetSubData(BufferId id, GLintptr offset, GLsizeiptr size, void* data);

	void BindBufferBase(BufferId id, GLenum target, GLuint index);
	void BindBufferRange(BufferId id, GLenum target, GLuint index, GLintptr offset, GLsizeiptr size);

#pragma endregion

	//-------------------------------------------------------------------------
	// BufferStorage
	//-------------------------------------------------------------------------
#pragma region [ BufferStorage ]

	/*
	EXAMPLE:
		static constexpr std::array<float, 6> triPositions = {-0, -0, 1, -1, 1, 1};
		CreateBuffer(triPositions);
	*/
	class TriviallyCopyableByteSpan final : public std::span<const std::byte>
	{
	public:
		template<typename T> requires std::is_trivially_copyable_v<T>
		TriviallyCopyableByteSpan(const T& t)
			: std::span<const std::byte>(std::as_bytes(std::span{ &t, static_cast<size_t>(1) }))
		{
		}

		template<typename T> requires std::is_trivially_copyable_v<T>
		TriviallyCopyableByteSpan(std::span<const T> t) : std::span<const std::byte>(std::as_bytes(t))
		{
		}

		template<typename T> requires std::is_trivially_copyable_v<T>
		TriviallyCopyableByteSpan(std::span<T> t) : std::span<const std::byte>(std::as_bytes(t))
		{
		}
	};

	struct BufferFillInfo final
	{
		uint64_t offset{ 0 };
		uint64_t size{ WHOLE_BUFFER };
		uint32_t data{ 0 };
	};

	BufferStorageId CreateStorageBuffer(size_t size, BufferStorageFlags storageFlags = BufferStorageFlag::NONE, std::string_view name = "");
	BufferStorageId CreateStorageBuffer(TriviallyCopyableByteSpan data, BufferStorageFlags storageFlags = BufferStorageFlag::NONE, std::string_view name = "");
	BufferStorageId CreateStorageBuffer(const void* data, size_t size, BufferStorageFlags storageFlags, std::string_view name);

	template<class T>
		requires(std::is_trivially_copyable_v<T>)
	BufferStorageId CreateStorageBuffer(BufferStorageFlags storageFlags = BufferStorageFlag::NONE, std::string_view name = "")
	{
		return CreateStorageBuffer(sizeof(T), storageFlags, name);
	}

	template<class T>
		requires(std::is_trivially_copyable_v<T>)
	BufferStorageId CreateStorageBuffer(size_t count, BufferStorageFlags storageFlags = BufferStorageFlag::NONE, std::string_view name = "")
	{
		return CreateStorageBuffer(sizeof(T) * count, storageFlags, name);
	}

	void UpdateData(BufferStorageId id, TriviallyCopyableByteSpan data, size_t destOffsetBytes = 0);
	void UpdateData(BufferStorageId id, const void* data, size_t size, size_t offset = 0);
	void FillData(BufferStorageId id, const BufferFillInfo& clear = {});

	[[nodiscard]] inline void* GetMappedPointer(BufferStorageId id) noexcept { return id.mappedMemory; }
	[[nodiscard]] inline bool IsMapped(BufferStorageId id) noexcept { return id.mappedMemory != nullptr; }
	void Invalidate(BufferStorageId id);

#pragma endregion

	//-------------------------------------------------------------------------
	// Vertex Array
	//-------------------------------------------------------------------------
#pragma region [ Vertex Array ]

	struct VertexInputBindingDescription final
	{
		uint32_t location; // glEnableVertexArrayAttrib + glVertexArrayAttribFormat
		uint32_t binding;  // glVertexArrayAttribBinding
		Format   format;   // glVertexArrayAttribFormat
		uint32_t offset;   // glVertexArrayAttribFormat
	};

	struct VertexInputState final
	{
		std::vector<VertexInputBindingDescription> vertexBindingDescriptions;
	};

	struct VertexAttributeRaw final // TODO: old, delete
	{
		//  TODO: для случая type = GL_INT не нужно передавать normalized. Подумать как сделать
		GLuint index;				// example: 0
		GLint  size;				// example: 3
		GLenum type;				// example: GL_FLOAT
		bool   normalized{ false };	// example: GL_FALSE
		GLuint relativeOffset;		// example: offsetof(Vertex, pos)
		GLuint bindingIndex{ 0 };
	};
	// example:
	//	SetVertexAttrib(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
	//	SetVertexAttrib(vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
	void SetVertexAttrib(GLuint vao, GLuint attribIndex, GLint size, GLenum type, GLboolean normalized, GLuint relativeOffset, GLuint bindingIndex); // TODO: old, delete
	void SetVertexAttrib(GLuint vao, const VertexAttributeRaw& attribute); // TODO: old, delete
	void SetVertexAttrib(GLuint vao, const std::vector<VertexAttributeRaw>& attributes); // TODO: old, delete

	VertexArrayId CreateVertexArray(); // TODO: old, delete
	VertexArrayId CreateVertexArray(const std::vector<VertexAttributeRaw>& attributes); // TODO: old, delete
	VertexArrayId CreateVertexArray(BufferId vbo, size_t vertexSize, const std::vector<VertexAttributeRaw>& attributes); // TODO: old, delete
	VertexArrayId CreateVertexArray(BufferId vbo, BufferId ibo, size_t vertexSize, const std::vector<VertexAttributeRaw>& attributes); // TODO: old, delete

	VertexArrayId CreateVertexArray(const VertexInputState& inputState);

	void SetVertexBuffer(VertexArrayId id, BufferId vbo, GLuint bindingindex, GLintptr offset, GLsizei stride);
	void SetIndexBuffer(VertexArrayId id, BufferId ibo);

	void Bind(VertexArrayId id);

#pragma endregion

	//-------------------------------------------------------------------------
	// Texture
	//-------------------------------------------------------------------------
#pragma region [ Texture ]

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

	void SetSubImage(Texture1DId id, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels);
	void SetSubImage(Texture2DId id, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
	void SetSubImage(Texture3DId id, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels);

	void SetCompressedSubImage(Texture1DId id, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data);
	void SetCompressedSubImage(Texture2DId id, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data);
	void SetCompressedSubImage(Texture3DId id, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data);

	void CopySubImage(Texture1DId id, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
	void CopySubImage(Texture2DId id, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	void CopySubImage(Texture3DId id, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

	void Bind(GLuint unit, Texture1DId id);
	void Bind(GLuint unit, Texture2DId id);
	void Bind(GLuint unit, Texture3DId id);
	void Bind(GLuint unit, TextureCubeId id);
#pragma endregion

	//-------------------------------------------------------------------------
	// New Texture
	//-------------------------------------------------------------------------
#pragma region [ (NEW) Texture ]

	struct TextureUpdateInfo final
	{
		uint32_t level{ 0 };
		Offset3D offset{};
		Extent3D extent{};
		UploadFormat format{ UploadFormat::INFER_FORMAT };
		UploadType type{ UploadType::INFER_TYPE };
		const void* pixels{ nullptr };

		// @brief Specifies, in texels, the size of rows in the array (for 2D and 3D images). If zero, it is assumed to be tightly packed according to size
		uint32_t rowLength{ 0 };

		// @brief Specifies, in texels, the number of rows in the array (for 3D images. If zero, it is assumed to be tightly packed according to size
		uint32_t imageHeight{ 0 };
	};

	struct CompressedTextureUpdateInfo final
	{
		uint32_t level{ 0 };
		Offset3D offset{};
		Extent3D extent{};
		const void* data{ nullptr };
	};

	struct TextureClearInfo final
	{
		uint32_t level{ 0 };
		Offset3D offset{};
		Extent3D extent{};
		UploadFormat format{ UploadFormat::INFER_FORMAT };
		UploadType type{ UploadType::INFER_TYPE };

		/// @brief If null, then the subresource will be cleared with zeroes
		const void* data{ nullptr };
	};

	// TODO: возможно всеже оставить разделение на 1D/2D/3D/Cube/etc?


	TextureId CreateTexture(const TextureCreateInfo& createInfo, std::string_view name = "");
	TextureId CreateTexture2D(Extent2D size, Format format, std::string_view name = "");
	TextureId CreateTexture2DMip(Extent2D size, Format format, uint32_t mipLevels, std::string_view name = "");

	void UpdateImage(TextureId id, const TextureUpdateInfo& info);
	void UpdateCompressedImage(TextureId id, const CompressedTextureUpdateInfo& info);
	void ClearImage(TextureId id, const TextureClearInfo& info);
	void GenMipmaps(TextureId id);

	// TODO: биндесс выделить в одтельный ресурс
	uint64_t GetBindlessHandle(TextureId id, SamplerId sampler);

#pragma endregion
	
	//-------------------------------------------------------------------------
	// TextureView
	//-------------------------------------------------------------------------
#pragma region [ TextureView ]

	TextureViewId CreateTextureView(const TextureViewCreateInfo& viewInfo, TextureId texture, std::string_view name = "");
	TextureViewId CreateTextureView(TextureId texture, std::string_view name = "");

	// Creates a view of a single mip level of the image
	TextureViewId CreateSingleMipView(TextureId texture, uint32_t level);
	// Creates a view of a single array layer of the image
	TextureViewId CreateSingleLayerView(TextureId texture, uint32_t layer);
	// Reinterpret the data of this texture
	TextureViewId CreateFormatView(TextureId texture, Format newFormat);
	// Creates a view of the texture with a new component mapping
	TextureViewId CreateSwizzleView(TextureId texture, ComponentMapping components);

#pragma endregion

	//-------------------------------------------------------------------------
	// Sampler
	//-------------------------------------------------------------------------
#pragma region [ Sampler ]

	struct SamplerState final
	{
		bool operator==(const SamplerState&) const noexcept = default;

		float lodBias{ 0 };
		float minLod{ -1000 };
		float maxLod{ 1000 };

		MinFilter minFilter{ MinFilter::Linear };
		MagFilter magFilter{ MagFilter::Linear };
		AddressMode addressModeU{ AddressMode::ClampToEdge };
		AddressMode addressModeV{ AddressMode::ClampToEdge };
		AddressMode addressModeW{ AddressMode::ClampToEdge };
		BorderColor borderColor{ BorderColor::FloatOpaqueWhite };
		SampleCount anisotropy{ SampleCount::Samples1 };
		bool compareEnable{ false };
		CompareOp compareOp{ CompareOp::Never };
	};

	SamplerId CreateSampler(const SamplerState& createInfo);
	void Bind(GLuint unit, SamplerId sampler);
	void Bind(GLuint unit, Texture2DId texture, SamplerId sampler);

#pragma endregion

	//-------------------------------------------------------------------------
	// RenderBuffer
	//-------------------------------------------------------------------------
#pragma region [ RenderBuffer ]

	RenderBufferId CreateRenderBuffer(GLenum internalFormat, GLsizei width, GLsizei height);
	RenderBufferId CreateRenderBuffer(GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height); // Multisample

#pragma endregion

	//-------------------------------------------------------------------------
	// FrameBuffer
	//-------------------------------------------------------------------------
#pragma region [ FrameBuffer ]

	GLuint CreateColorBuffer2D(int width, int height, GLenum formatColor); // удалить - через создание текстуры
	GLuint CreateDepthBuffer2D(int width, int height, GLenum formatDepth = GL_DEPTH_COMPONENT32);
	// TODO: CreateRenderBuffer???
	FrameBufferId CreateFrameBuffer2D(GLuint colorBuffer, GLuint depthBuffer); // TODO:

	FrameBufferId CreateFrameBuffer2D(Texture2DId colorBuffer, Texture2DId depthBuffer);
	FrameBufferId CreateFrameBuffer2D(Texture2DId colorBuffer, RenderBufferId depthBuffer);

	void SetDrawBuffer(FrameBufferId fbo, GLenum buffer);
	void SetDrawBuffers(FrameBufferId fbo, GLsizei size, const GLenum* buffers);

	void Invalidate(FrameBufferId fbo, GLsizei numAttachments, const GLenum* attachments);
	void InvalidateSubData(FrameBufferId fbo, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height);

	void SetFrameBuffer(FrameBufferId fbo);
	void SetFrameBuffer(FrameBufferId fbo, int width, int height, GLbitfield clearMask);

#pragma endregion

	//-------------------------------------------------------------------------
	// NEW FrameBuffer
	//-------------------------------------------------------------------------
#pragma region [ (NEW) FrameBuffer ]

	// Describes the render targets that may be used in a draw
	struct FrameBufferCreateInfo final
	{
		std::vector<TextureId> colorAttachments{};
		std::optional<TextureId> depthAttachment{ std::nullopt };
		std::optional<TextureId> stencilAttachment{ std::nullopt };
	};
	// TODO: также еще и с RenderBufferId

	FrameBufferId CreateFrameBuffer(const FrameBufferCreateInfo& renderInfo);

#pragma endregion

	//-------------------------------------------------------------------------
	// GraphicsPipeline
	//-------------------------------------------------------------------------
#pragma region [ GraphicsPipeline ]

	struct InputAssemblyState final
	{
		PrimitiveTopology topology = PrimitiveTopology::TRIANGLE_LIST;
		bool primitiveRestartEnable = false;

		bool operator==(const InputAssemblyState&) const noexcept = default;
	};

	struct TessellationState final
	{
		uint32_t patchControlPoints; // glPatchParameteri(GL_PATCH_VERTICES, ...)

		bool operator==(const TessellationState&) const noexcept = default;
	};

	struct RasterizationState final
	{
		bool        depthClampEnable{ false };
		PolygonMode polygonMode{ PolygonMode::Fill };
		CullMode    cullMode{ CullMode::Back };
		FrontFace   frontFace{ FrontFace::CounterClockwise };
		bool        depthBiasEnable{ false };
		float       depthBiasConstantFactor{ 0 };
		float       depthBiasSlopeFactor{ 0 };
		float       lineWidth{ 1 }; // glLineWidth
		float       pointSize{ 1 }; // glPointSize

		bool operator==(const RasterizationState&) const noexcept = default;
	};

	struct MultisampleState final
	{
		bool sampleShadingEnable{ false };   // glEnable(GL_SAMPLE_SHADING)
		float minSampleShading{ 1 };         // glMinSampleShading
		uint32_t sampleMask{ 0xFFFFFFFF };   // glSampleMaski
		bool alphaToCoverageEnable{ false }; // glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE)
		bool alphaToOneEnable{ false };      // glEnable(GL_SAMPLE_ALPHA_TO_ONE)

		bool operator==(const MultisampleState&) const noexcept = default;
	};

	struct DepthState final
	{
		bool depthTestEnable{ false };               // gl{Enable, Disable}(GL_DEPTH_TEST)
		bool depthWriteEnable{ false };              // glDepthMask(depthWriteEnable)
		CompareOp depthCompareOp{ CompareOp::Less }; // glDepthFunc

		bool operator==(const DepthState&) const noexcept = default;
	};

	struct StencilOpState final
	{
		StencilOp passOp{ StencilOp::Keep };      // glStencilOp (dppass)
		StencilOp failOp{ StencilOp::Keep };      // glStencilOp (sfail)
		StencilOp depthFailOp{ StencilOp::Keep }; // glStencilOp (dpfail)
		CompareOp compareOp{ CompareOp::Always }; // glStencilFunc (func)
		uint32_t compareMask{ 0 };                // glStencilFunc (mask)
		uint32_t writeMask{ 0 };                  // glStencilMask
		uint32_t reference{ 0 };                  // glStencilFunc (ref)

		bool operator==(const StencilOpState&) const noexcept = default;
	};

	struct StencilState final
	{
		bool stencilTestEnable{ false };
		StencilOpState front{};
		StencilOpState back{};

		bool operator==(const StencilState&) const noexcept = default;
	};

	struct ColorBlendAttachmentState final // glBlendFuncSeparatei + glBlendEquationSeparatei
	{
		bool blendEnable{ false };                                           // if false, blend factor = one?
		BlendFactor srcColorBlendFactor{ BlendFactor::One };              // srcRGB
		BlendFactor dstColorBlendFactor{ BlendFactor::Zero };             // dstRGB
		BlendOp colorBlendOp{ BlendOp::Add };                  // modeRGB
		BlendFactor srcAlphaBlendFactor{ BlendFactor::One };              // srcAlpha
		BlendFactor dstAlphaBlendFactor{ BlendFactor::Zero };             // dstAlpha
		BlendOp alphaBlendOp{ BlendOp::Add };                  // modeAlpha
		ColorComponentFlags colorWriteMask{ ColorComponentFlag::RGBA_BITS }; // glColorMaski

		bool operator==(const ColorBlendAttachmentState&) const noexcept = default;
	};

	struct ColorBlendState final
	{
		bool logicOpEnable{ false };          // gl{Enable, Disable}(GL_COLOR_LOGIC_OP)
		LogicOp logicOp{ LogicOp::Copy };  // glLogicOp(logicOp)
		std::vector<ColorBlendAttachmentState> attachments{};             // glBlendFuncSeparatei + glBlendEquationSeparatei
		float blendConstants[4] = { 0, 0, 0, 0 }; // glBlendColor

		bool operator==(const ColorBlendState&) const noexcept = default;
	};

	// Parameters for the constructor of GraphicsPipeline
	struct GraphicsPipelineCreateInfo final
	{
		std::string_view   debugName;

		std::string_view   vertexShader{ "" };
		std::string_view   fragmentShader{ "" };
		std::string_view   tessellationControlShader{ "" };
		std::string_view   tessellationEvaluationShader{ "" };

		InputAssemblyState inputAssemblyState{};
		VertexInputState   vertexInputState{};
		TessellationState  tessellationState{};
		RasterizationState rasterizationState {};
		MultisampleState   multisampleState{};
		DepthState         depthState{};
		StencilState       stencilState{};
		ColorBlendState    colorBlendState{};
	};

	struct GraphicsPipelineId final
	{
		std::string_view   debugName;

		ShaderProgramId    program;
		VertexArrayId      vao;
		InputAssemblyState inputAssemblyState;
		VertexInputState   vertexInputState;
		TessellationState  tessellationState;
		RasterizationState rasterizationState;
		MultisampleState   multisampleState;
		DepthState         depthState;
		StencilState       stencilState;
		ColorBlendState    colorBlendState;
		std::vector<std::pair<std::string, uint32_t>> uniformBlocks;
		std::vector<std::pair<std::string, uint32_t>> storageBlocks;
		std::vector<std::pair<std::string, uint32_t>> samplersAndImages;

		bool valid{ false };
	};

	GraphicsPipelineId CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo);

	template<>
	inline void Destroy(GraphicsPipelineId& res)
	{
		Destroy(res.program);
		Destroy(res.vao);
		res.uniformBlocks.clear();
		res.storageBlocks.clear();
		res.samplersAndImages.clear();
	}

#pragma endregion

	//-------------------------------------------------------------------------
	// ComputePipeline
	//-------------------------------------------------------------------------
#pragma region [ ComputePipeline ]

	struct ComputePipelineInfo final
	{
		std::string_view debugName;
		std::string_view shader;
	};

	struct ComputePipelineId final
	{
		ShaderProgramId program;
		Extent3D        workgroupSize;
		std::vector<std::pair<std::string, uint32_t>> uniformBlocks;
		std::vector<std::pair<std::string, uint32_t>> storageBlocks;
		std::vector<std::pair<std::string, uint32_t>> samplersAndImages;

		bool            valid{ false };
	};

	ComputePipelineId CreateComputePipeline(const ComputePipelineInfo& createInfo);

	template<>
	inline void Destroy(ComputePipelineId& res)
	{
		Destroy(res.program);
		res.uniformBlocks.clear();
		res.storageBlocks.clear();
		res.samplersAndImages.clear();
	}

#pragma endregion

	//-------------------------------------------------------------------------
	// RenderInfo
	//-------------------------------------------------------------------------
#pragma region [ RenderInfo ]

	struct Viewport final
	{
		Rect2D drawRect{};  // glViewport
		float minDepth{ 0.0f }; // glDepthRangef
		float maxDepth{ 1.0f }; // glDepthRangef
		ClipDepthRange depthRange = // glClipControl
#ifdef SE_DEFAULT_CLIP_DEPTH_RANGE_NEGATIVE_ONE_TO_ONE
			ClipDepthRange::NEGATIVE_ONE_TO_ONE;
#else
			ClipDepthRange::ZERO_TO_ONE;
#endif

		bool operator==(const Viewport&) const noexcept = default;
	};

	// Tells what to do with a render target at the beginning of a pass
	enum class AttachmentLoadOp : uint32_t
	{
		// The previous contents of the image will be preserved
		Load,
		// The contents of the image will be cleared to a uniform value
		Clear,
		// The previous contents of the image need not be preserved (they may be discarded)
		DontCare,
	};

	struct RenderColorAttachment final
	{
		TextureId texture;
		AttachmentLoadOp loadOp = AttachmentLoadOp::Load;
		glm::vec4 clearValue;
	};

	struct ClearDepthStencilValue final
	{
		float depth{};
		int32_t stencil{};
	};

	struct RenderDepthStencilAttachment final
	{
		TextureId texture;
		AttachmentLoadOp loadOp = AttachmentLoadOp::Load;
		ClearDepthStencilValue clearValue;
	};


	// Describes the render targets that may be used in a draw
	struct RenderInfo final
	{
		/// @brief An optional name to demarcate the pass in a graphics debugger
		std::string_view name;

		/// @brief An optional viewport
		/// 
		/// If empty, the viewport size will be the minimum the render targets' size and the offset will be 0.
		std::optional<Viewport> viewport = std::nullopt;
		std::span<const RenderColorAttachment> colorAttachments;
		std::optional<RenderDepthStencilAttachment> depthAttachment = std::nullopt;
		std::optional<RenderDepthStencilAttachment> stencilAttachment = std::nullopt;
	};


#pragma endregion

	//-------------------------------------------------------------------------
	// Cmd
	//-------------------------------------------------------------------------
#pragma region [ Cmd ]

	namespace Cmd
	{
		//=====================================================================
		// Rendering scopes
		//=====================================================================

		// Binds a graphics pipeline to be used for future draw operations
		void BindGraphicsPipeline(const GraphicsPipelineId& pipeline, bool resetCacheState = false);

		// Binds a compute pipeline to be used for future dispatch operations
		void BindComputePipeline(const ComputePipelineId& pipeline);

		// Dynamically sets the viewport
		// Similar to glViewport
		void SetViewport(const Viewport& viewport);

		// Dynamically sets the scissor rect
		/// Similar to glScissor.
		void SetScissor(const Rect2D& scissor);

		// Equivalent to glDrawArraysInstancedBaseInstance or vkCmdDraw
		// - vertexCount The number of vertices to draw
		// - instanceCount The number of instances to draw
		// - firstVertex The index of the first vertex to draw
		// - firstInstance The instance ID of the first instance to draw
		void Draw(
			uint32_t vertexCount,
			uint32_t instanceCount,
			uint32_t firstVertex,
			uint32_t firstInstance);

		// Equivalent to glDrawElementsInstancedBaseVertexBaseInstance or vkCmdDrawIndexed
		// - indexCount The number of vertices to draw
		// - instanceCount The number of instances to draw
		// - firstIndex The base index within the index buffer
		// - vertexOffset The value added to the vertex index before indexing into the vertex buffer
		// - firstInstance The instance ID of the first instance to draw
		void DrawIndexed(
			uint32_t indexCount,
			uint32_t instanceCount,
			uint32_t firstIndex,
			int32_t vertexOffset,
			uint32_t firstInstance);

		// Equivalent to glMultiDrawArraysIndirect or vkCmdDrawDrawIndirect
		// - commandBuffer The buffer containing draw parameters
		// - commandBufferOffset The byte offset into commandBuffer where parameters begin
		// - drawCount The number of draws to execute
		// - stride The byte stride between successive sets of draw parameters
		void DrawIndirect(
			const BufferStorageId& commandBuffer,
			uint64_t commandBufferOffset,
			uint32_t drawCount,
			uint32_t stride);

		// Equivalent to glMultiDrawArraysIndirectCount or vkCmdDrawIndirectCount
		// - commandBuffer The buffer containing draw parameters
		// - commandBufferOffset The byte offset into commandBuffer where parameters begin
		// - countBuffer The buffer containing the draw count
		// - countBufferOffset The byte offset into countBuffer where the draw count begins
		// - maxDrawCount The maximum number of draws that will be executed
		// - stride The byte stride between successive sets of draw parameters
		void DrawIndirectCount(
			const BufferStorageId& commandBuffer,
			uint64_t commandBufferOffset,
			const BufferStorageId& countBuffer,
			uint64_t countBufferOffset,
			uint32_t maxDrawCount,
			uint32_t stride);

		// Equivalent to glMultiDrawElementsIndirect or vkCmdDrawIndexedIndirect
		// - commandBuffer The buffer containing draw parameters
		// - commandBufferOffset The byte offset into commandBuffer where parameters begin
		// - drawCount The number of draws to execute
		// - stride The byte stride between successive sets of draw parameters
		void DrawIndexedIndirect(
			const BufferStorageId& commandBuffer,
			uint64_t commandBufferOffset,
			uint32_t drawCount,
			uint32_t stride);

		// Equivalent to glMultiDrawElementsIndirectCount or vkCmdDrawIndexedIndirectCount
		// - commandBuffer The buffer containing draw parameters
		// - commandBufferOffset The byte offset into commandBuffer where parameters begin
		// - countBuffer The buffer containing the draw count
		// - countBufferOffset The byte offset into countBuffer where the draw count begins
		// - maxDrawCount The maximum number of draws that will be executed
		// - stride The byte stride between successive sets of draw parameters
		void DrawIndexedIndirectCount(
			const BufferStorageId& commandBuffer,
			uint64_t commandBufferOffset,
			const BufferStorageId& countBuffer,
			uint64_t countBufferOffset,
			uint32_t maxDrawCount,
			uint32_t stride);

		// Binds a buffer to a vertex buffer binding point
		// Similar to glVertexArrayVertexBuffer.
		void BindVertexBuffer(uint32_t bindingIndex, const BufferStorageId& buffer, uint64_t offset, uint64_t stride);
		// Binds an index buffer
		// Similar to glVertexArrayElementBuffer.
		void BindIndexBuffer(const BufferStorageId& buffer, IndexType indexType);

		// Binds a range within a buffer as a uniform buffer
		// Similar to glBindBufferRange(GL_UNIFORM_BUFFER, ...)
		void BindUniformBuffer(uint32_t index, const BufferStorageId& buffer, uint64_t offset = 0, uint64_t size = WHOLE_BUFFER);

		// Binds a range within a buffer as a uniform buffer
		// - block The name of the uniform block whose index to bind to
		// - note: Must be called after a pipeline is bound in order to get reflected program info
		void BindUniformBuffer(std::string_view block, const BufferStorageId& buffer, uint64_t offset = 0, uint64_t size = WHOLE_BUFFER);

		// Binds a range within a buffer as a storage buffer
		// Similar to glBindBufferRange(GL_SHADER_STORAGE_BUFFER, ...)
		void BindStorageBuffer(uint32_t index, const BufferStorageId& buffer, uint64_t offset = 0, uint64_t size = WHOLE_BUFFER);

		// Binds a range within a buffer as a storage buffer
		// - block The name of the storage block whose index to bind to
		// - note: Must be called after a pipeline is bound in order to get reflected program info
		void BindStorageBuffer(std::string_view block, const BufferStorageId& buffer, uint64_t offset = 0, uint64_t size = WHOLE_BUFFER);

		// Binds a texture and a sampler to a texture unit
		// Similar to glBindTextureUnit + glBindSampler
		void BindSampledImage(uint32_t index, const TextureId& texture, const SamplerId& sampler);

		// Binds a texture and a sampler to a texture unit
		// - uniform The name of the uniform whose index to bind to
		// - note: Must be called after a pipeline is bound in order to get reflected program info
		void BindSampledImage(std::string_view uniform, const TextureId& texture, const SamplerId& sampler);

		// Binds a texture to an image unit
		// Similar to glBindImageTexture{s}
		void BindImage(uint32_t index, const TextureId& texture, uint32_t level);

		// Binds a texture to an image unit
		// - uniform The name of the uniform whose index to bind to
		// - note: Must be called after a pipeline is bound in order to get reflected program info
		void BindImage(std::string_view uniform, const TextureId& texture, uint32_t level);

		//=====================================================================
		// Compute scopes
		//=====================================================================

		// Invokes a compute shader
		// - groupCountX The number of local workgroups to dispatch in the X dimension
		// - groupCountY The number of local workgroups to dispatch in the Y dimension
		// - groupCountZ The number of local workgroups to dispatch in the Z dimension
		void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

		// Invokes a compute shader
		// - groupCount The number of local workgroups to dispatch
		void Dispatch(Extent3D groupCount);

		// Invokes a compute shader a specified number of times
		// - invocationCountX The minimum number of invocations in the X dimension
		// - invocationCountY The minimum number of invocations in the Y dimension
		// - invocationCountZ The minimum number of invocations in the Z dimension
		// Automatically computes the number of workgroups to invoke based on the formula 
		// groupCount = (invocationCount + workgroupSize - 1) / workgroupSize.
		void DispatchInvocations(uint32_t invocationCountX, uint32_t invocationCountY, uint32_t invocationCountZ);

		// Invokes a compute shader a specified number of times
		// - invocationCount The minimum number of invocations
		// Automatically computes the number of workgroups to invoke based on the formula 
		// groupCount = (invocationCount + workgroupSize - 1) / workgroupSize.
		void DispatchInvocations(Extent3D invocationCount);

		// Invokes a compute shader with at least as many threads as there are pixels in the image
		// - texture The texture from which to infer the dispatch size
		// - lod The level of detail of the tetxure from which to infer the dispatch size
		// Automatically computes the number of workgroups to invoke based on the formula
		// groupCount = (invocationCount + workgroupSize - 1) / workgroupSize.
		// For 3D images, the depth is used for the Z component of invocationCount. Otherwise, the number of array layers will be used.
		// For cube textures, the Z component of invocationCount will be equal to 6 times the number of array layers.
		void DispatchInvocations(const TextureId& texture, uint32_t lod = 0);

		// Invokes a compute shader with the group count provided by a buffer
		// - commandBuffer The buffer containing dispatch parameters
		// - commandBufferOffset The byte offset into commandBuffer where the parameters begin
		void DispatchIndirect(const BufferStorageId& commandBuffer, uint64_t commandBufferOffset);
	} // namespace Cmd

#pragma endregion

} // namespace gl4