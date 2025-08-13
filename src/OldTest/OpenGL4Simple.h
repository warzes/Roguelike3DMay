#pragma once

#include "Engine/OpenGL4ApiToEnum.h"
#include "Engine/OpenGL4Shader.h"
#include "Engine/OpenGL4Buffer.h"
#include "Engine/OpenGL4Texture.h"
#include "Engine/OpenGL4Sampler.h"
#include "Engine/OpenGL4Pipeline.h"
#include "Engine/OpenGL4Render.h"

// Остается чистой и минимальной оберткой над OpenGL
// TODO: почистить

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

namespace gl
{
	
	//-------------------------------------------------------------------------
	// OpenGL RHI Types
	//-------------------------------------------------------------------------
#pragma region [ OpenGL RHI Types ]

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
		return CreateBufferStorage(flags, sizeof(T), (GLsizeiptr)data.size(), data.data());
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

	//BufferStorageId CreateStorageBuffer(size_t size, BufferStorageFlags storageFlags = BufferStorageFlag::None, std::string_view name = "");
	//BufferStorageId CreateStorageBuffer(TriviallyCopyableByteSpan data, BufferStorageFlags storageFlags = BufferStorageFlag::None, std::string_view name = "");
	//BufferStorageId CreateStorageBuffer(const void* data, size_t size, BufferStorageFlags storageFlags, std::string_view name);

	template<class T>
		requires(std::is_trivially_copyable_v<T>)
	BufferStorageId CreateStorageBuffer(BufferStorageFlags storageFlags = BufferStorageFlag::None, std::string_view name = "")
	{
		return CreateStorageBuffer(sizeof(T), storageFlags, name);
	}

	template<class T>
		requires(std::is_trivially_copyable_v<T>)
	BufferStorageId CreateStorageBuffer(size_t count, BufferStorageFlags storageFlags = BufferStorageFlag::None, std::string_view name = "")
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

	//VertexArrayId CreateVertexArray(const VertexInputState& inputState);

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
	// Sampler
	//-------------------------------------------------------------------------
#pragma region [ Sampler ]

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

} // namespace gl