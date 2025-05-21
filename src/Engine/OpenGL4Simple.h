#pragma once

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
	// Base
	//-------------------------------------------------------------------------
#pragma region [ Base ]

	// multisampling and anisotropy
	enum class SampleCount : uint8_t
	{
		Samples1  = 1,
		Samples2  = 2,
		Samples4  = 4,
		Samples8  = 8,
		Samples16 = 16,
		Samples32 = 32,
	};
	inline GLsizei EnumToGL(SampleCount sampleCount)
	{
		switch (sampleCount)
		{
		case SampleCount::Samples1:  return 1;
		case SampleCount::Samples2:  return 2;
		case SampleCount::Samples4:  return 4;
		case SampleCount::Samples8:  return 8;
		case SampleCount::Samples16: return 16;
		case SampleCount::Samples32: return 32;
		default: assert(0);          return 0;
		}
	}

	enum class MagFilter : uint8_t
	{
		Nearest,
		Linear
	};
	inline GLenum EnumToGL(MagFilter filter)
	{
		switch (filter)
		{
		case MagFilter::Nearest: return GL_NEAREST;
		case MagFilter::Linear:  return GL_LINEAR;
		default: assert(0); return 0;
		}
	}

	enum class MinFilter : uint8_t
	{
		Nearest,
		Linear,

		NearestMimapNearest,
		NearestMimapLinear,
		LinearMimapNearest,
		LinearMimapLinear
	};
	inline GLenum EnumToGL(MinFilter filter)
	{
		switch (filter)
		{
		case MinFilter::Nearest: return GL_NEAREST;
		case MinFilter::Linear:  return GL_LINEAR;
		case MinFilter::NearestMimapNearest: return GL_NEAREST_MIPMAP_NEAREST;
		case MinFilter::NearestMimapLinear: return GL_NEAREST_MIPMAP_LINEAR;
		case MinFilter::LinearMimapNearest: return GL_LINEAR_MIPMAP_NEAREST;
		case MinFilter::LinearMimapLinear: return GL_LINEAR_MIPMAP_LINEAR;

		default: assert(0); return 0;
		}
	}

	enum class AddressMode : uint8_t
	{
		Repeat,
		MirroredRepeat,
		ClampToEdge,
		ClampToBorder,
		MirrorClampToEdge,
	};
	inline GLint EnumToGL(AddressMode addressMode)
	{
		switch (addressMode)
		{
		case AddressMode::Repeat:            return GL_REPEAT;
		case AddressMode::MirroredRepeat:    return GL_MIRRORED_REPEAT;
		case AddressMode::ClampToEdge:       return GL_CLAMP_TO_EDGE;
		case AddressMode::ClampToBorder:     return GL_CLAMP_TO_BORDER;
		case AddressMode::MirrorClampToEdge: return GL_MIRROR_CLAMP_TO_EDGE;
		default: assert(0);                  return 0;
		}
	}

	enum class BorderColor : uint8_t
	{
		FloatTransparentBlack,
		IntTransparentBlack,
		FloatOpaqueBlack,
		IntOpaqueBlack,
		FloatOpaqueWhite,
		IntOpaqueWhite,
	};

	enum class CompareOp : uint8_t
	{
		Never,
		Less,
		Equal,
		LessOrEqual,
		Greater,
		NotEqual,
		GreaterOrEqual,
		Always,
	};
	inline GLenum EnumToGL(CompareOp op)
	{
		switch (op)
		{
		case CompareOp::Never:          return GL_NEVER;
		case CompareOp::Less:           return GL_LESS;
		case CompareOp::Equal:          return GL_EQUAL;
		case CompareOp::LessOrEqual:    return GL_LEQUAL;
		case CompareOp::Greater:        return GL_GREATER;
		case CompareOp::NotEqual:       return GL_NOTEQUAL;
		case CompareOp::GreaterOrEqual: return GL_GEQUAL;
		case CompareOp::Always:         return GL_ALWAYS;
		default: assert(0);             return 0;
		}
	}

	enum class StencilOp : uint8_t
	{
		Keep,
		Zero,
		Replace,
		IncrementAndClamp,
		DecrementAndClamp,
		Invert,
		IncrementAndWrap,
		DecrementAndWrap,
	};
	inline GLenum EnumToGL(StencilOp op)
	{
		switch (op)
		{
		case StencilOp::Keep: return GL_KEEP;
		case StencilOp::Zero: return GL_ZERO;
		case StencilOp::Replace: return GL_REPLACE;
		case StencilOp::IncrementAndClamp: return GL_INCR;
		case StencilOp::DecrementAndClamp: return GL_DECR;
		case StencilOp::Invert: return GL_INVERT;
		case StencilOp::IncrementAndWrap: return GL_INCR_WRAP;
		case StencilOp::DecrementAndWrap: return GL_DECR_WRAP;
		default: assert(0); return 0;
		}
	}

	enum class PolygonMode : uint8_t
	{
		Fill,
		Line,
		Point,
	};
	inline GLenum EnumToGL(PolygonMode mode)
	{
		switch (mode)
		{
		case PolygonMode::Fill: return GL_FILL;
		case PolygonMode::Line: return GL_LINE;
		case PolygonMode::Point: return GL_POINT;
		default: assert(0); return 0;
		}
	}

	enum class CullMode : uint8_t
	{
		Front,
		Back,
		FrontAndBack,
	};
	inline GLenum EnumToGL(CullMode mode)
	{
		switch (mode)
		{
		case CullMode::Front: return GL_FRONT;
		case CullMode::Back: return GL_BACK;
		case CullMode::FrontAndBack: return GL_FRONT_AND_BACK;
		default: assert(0); return 0;
		}
	}

	enum class FrontFace : uint8_t
	{
		Clockwise,
		CounterClockwise,
	};
	inline GLenum EnumToGL(FrontFace face)
	{
		switch (face)
		{
		case FrontFace::Clockwise: return GL_CW;
		case FrontFace::CounterClockwise: return GL_CCW;
		default: assert(0); return 0;
		}
	}

	enum class BlendFactor : uint8_t
	{
		Zero,
		One,
		SrcColor,
		OneMinusSrcColor,
		DstColor,
		OneMinusDstColor,
		SrcAlpha,
		OneMinusSrcAlpha,
		DstAlpha,
		OneMinusDstAlpha,
		ConstantColor,
		OneMinusConstantColor,
		ConstantAlpha,
		OneMinusConstantAlpha,
		SrcAlphaSaturate,
		Src1Color,
		OneMinusSrc1Color,
		Src1Alpha,
		OneMinusSrc1Alpha,
	};
	inline GLenum EnumToGL(BlendFactor factor)
	{
		switch (factor)
		{
		case BlendFactor::Zero: return GL_ZERO;
		case BlendFactor::One: return GL_ONE;
		case BlendFactor::SrcColor: return GL_SRC_COLOR;
		case BlendFactor::OneMinusSrcColor: return GL_ONE_MINUS_SRC_COLOR;
		case BlendFactor::DstColor: return GL_DST_COLOR;
		case BlendFactor::OneMinusDstColor: return GL_ONE_MINUS_DST_COLOR;
		case BlendFactor::SrcAlpha: return GL_SRC_ALPHA;
		case BlendFactor::OneMinusSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
		case BlendFactor::DstAlpha: return GL_DST_ALPHA;
		case BlendFactor::OneMinusDstAlpha: return GL_ONE_MINUS_DST_ALPHA;
		case BlendFactor::ConstantColor: return GL_CONSTANT_COLOR;
		case BlendFactor::OneMinusConstantColor: return GL_ONE_MINUS_CONSTANT_COLOR;
		case BlendFactor::ConstantAlpha: return GL_CONSTANT_ALPHA;
		case BlendFactor::OneMinusConstantAlpha: return GL_ONE_MINUS_CONSTANT_ALPHA;
		case BlendFactor::SrcAlphaSaturate: return GL_SRC_ALPHA_SATURATE;
		case BlendFactor::Src1Color: return GL_SRC1_COLOR;
		case BlendFactor::OneMinusSrc1Color: return GL_ONE_MINUS_SRC1_COLOR;
		case BlendFactor::Src1Alpha: return GL_SRC1_ALPHA;
		case BlendFactor::OneMinusSrc1Alpha: return GL_ONE_MINUS_SRC1_ALPHA;
		default: assert(0); return 0;
		}
	}

	enum class BlendOp : uint8_t
	{
		Add,
		Subtract,
		ReverseSubtract,
		Min,
		Max,
	};
	inline GLenum EnumToGL(BlendOp op)
	{
		switch (op)
		{
		case BlendOp::Add: return GL_FUNC_ADD;
		case BlendOp::Subtract: return GL_FUNC_SUBTRACT;
		case BlendOp::ReverseSubtract: return GL_FUNC_REVERSE_SUBTRACT;
		case BlendOp::Min: return GL_MIN;
		case BlendOp::Max: return GL_MAX;
		default: assert(0); return 0;
		}
	}

	enum class LogicOp : uint8_t
	{
		Clear,
		Set,
		Copy,
		CopyInverted,
		NoOp,
		Invert,
		And,
		Nand,
		Or,
		Nor,
		Xor,
		Equivalent,
		AndReverse,
		OrReverse,
		AndInverted,
		OrInverted
	};
	inline GLenum EnumToGL(LogicOp op)
	{
		switch (op)
		{
		case LogicOp::Clear: return GL_CLEAR;
		case LogicOp::Set: return GL_SET;
		case LogicOp::Copy: return GL_COPY;
		case LogicOp::CopyInverted: return GL_COPY_INVERTED;
		case LogicOp::NoOp: return GL_NOOP;
		case LogicOp::Invert: return GL_INVERT;
		case LogicOp::And: return GL_AND;
		case LogicOp::Nand: return GL_NAND;
		case LogicOp::Or: return GL_OR;
		case LogicOp::Nor: return GL_NOR;
		case LogicOp::Xor: return GL_XOR;
		case LogicOp::Equivalent: return GL_EQUIV;
		case LogicOp::AndReverse: return GL_AND_REVERSE;
		case LogicOp::OrReverse: return GL_OR_REVERSE;
		case LogicOp::AndInverted: return GL_AND_INVERTED;
		case LogicOp::OrInverted: return GL_OR_INVERTED;
		default: assert(0); return 0;
		}
	}

#pragma endregion

	//-------------------------------------------------------------------------
	// OpenGL RHI Types
	//-------------------------------------------------------------------------
#pragma region [ OpenGL RHI Types ]

	constexpr inline uint64_t WHOLE_BUFFER = static_cast<uint64_t>(-1);

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

	template<typename T>
	bool IsValid(T res) { return res.id > 0; }

	template<typename T>
	void Create(T& res)
	{
		assert(!res.id);

		if constexpr (std::is_same_v<T, ShaderProgramId>) { res.id = glCreateProgram(); }
		else if constexpr (std::is_same_v<T, BufferId>) { glCreateBuffers(1, &res.id); }
		else if constexpr (std::is_same_v<T, VertexArrayId>) { glCreateVertexArrays(1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture1DId>) { glCreateTextures(GL_TEXTURE_1D, 1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture2DId>) { glCreateTextures(GL_TEXTURE_2D, 1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture3DId>) { glCreateTextures(GL_TEXTURE_3D, 1, &res.id); }
		else if constexpr (std::is_same_v<T, TextureCubeId>) { glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture1DArrayId>) { glCreateTextures(GL_TEXTURE_1D_ARRAY, 1, &res.id); }
		else if constexpr (std::is_same_v<T, Texture2DArrayId>) { glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &res.id); }
		else if constexpr (std::is_same_v<T, TextureCubeArrayId>) { glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &res.id); }		
		else if constexpr (std::is_same_v<T, SamplerId>) { glCreateSamplers(1, &res.id); }
		else if constexpr (std::is_same_v<T, RenderBufferId>) { glCreateRenderbuffers(1, &res.id); }
		else if constexpr (std::is_same_v<T, FrameBufferId>) { glCreateFramebuffers(1, &res.id); }
		assert(res.id);
	}

	template<typename T>
	void Destroy(T& res)
	{
		if (res.id == 0) return;

		if constexpr (std::is_same_v<T, ShaderProgramId>) { glDeleteProgram(res.id); }
		else if constexpr (std::is_same_v<T, BufferId>) { glDeleteBuffers(1, &res.id); }
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

#pragma endregion
		
	//-------------------------------------------------------------------------
	// Shader
	//-------------------------------------------------------------------------
#pragma region [ Shader ]

	struct SpecializationConstant final
	{
		uint32_t index{ 0 };
		uint32_t value{ 0 };
	};

	struct ShaderSpirvInfo final
	{
		const char* entryPoint = "main";
		std::span<const uint32_t> code;
		std::span<const SpecializationConstant> specializationConstants;
	};

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
	// Buffer
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
	// Vertex Array
	//-------------------------------------------------------------------------
#pragma region [ Vertex Array ]

	struct VertexAttribute final
	{
		//  TODO: для случая type = GL_INT не нужно передавать normalized. Подумать как сделать
		GLuint index;				// example: 0
		GLint size;					// example: 3
		GLenum type;				// example: GL_FLOAT
		bool normalized{ false };	// example: GL_FALSE
		GLuint relativeOffset;		// example: offsetof(Vertex, pos)
		GLuint bindingIndex{ 0 };
	};

	// example:
	//	SetVertexAttrib(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
	//	SetVertexAttrib(vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
	void SetVertexAttrib(GLuint vao, GLuint attribIndex, GLint size, GLenum type, GLboolean normalized, GLuint relativeOffset, GLuint bindingIndex);
	void SetVertexAttrib(GLuint vao, const VertexAttribute& attribute);
	void SetVertexAttrib(GLuint vao, const std::vector<VertexAttribute>& attributes);

	VertexArrayId CreateVertexArray();
	VertexArrayId CreateVertexArray(const std::vector<VertexAttribute>& attributes);
	VertexArrayId CreateVertexArray(BufferId vbo, size_t vertexSize, const std::vector<VertexAttribute>& attributes);
	VertexArrayId CreateVertexArray(BufferId vbo, BufferId ibo, size_t vertexSize, const std::vector<VertexAttribute>& attributes);

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

	void SetFrameBuffer(gl4::FrameBufferId fbo);
	void SetFrameBuffer(gl4::FrameBufferId fbo, int width, int height, GLbitfield clearMask);

#pragma endregion

	//-------------------------------------------------------------------------
	// State
	//-------------------------------------------------------------------------
#pragma region [ State ]
	
	void SwitchDepthTestState(bool state);
	void SwitchBlendingState(bool state);

	void SwitchPolygonMode(PolygonMode mode);
	void SwitchDepthTestFunc(CompareOp mode);
	void SwitchBlendingFunc(BlendFactor mode);

#pragma endregion

} // namespace gl4