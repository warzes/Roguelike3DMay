#pragma once

#include "BasicTypes.h"

#define SE_DEFAULT_CLIP_DEPTH_RANGE_NEGATIVE_ONE_TO_ONE 1

namespace gl4
{
	// multisampling and anisotropy
	enum class SampleCount : uint8_t
	{
		Samples1 = 1,
		Samples2 = 2,
		Samples4 = 4,
		Samples8 = 8,
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
		None,
		Front,
		Back,
		FrontAndBack,
	};
	inline GLenum EnumToGL(CullMode mode)
	{
		switch (mode)
		{
		case CullMode::None: return 0;
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

	enum class GlFormatClass : uint8_t
	{
		Float,
		Int,
		Long
	};

	enum class Format : uint8_t
	{
		UNDEFINED,

		// Color formats
		R8_UNORM,
		R8_SNORM,
		R16_UNORM,
		R16_SNORM,
		R8G8_UNORM,
		R8G8_SNORM,
		R16G16_UNORM,
		R16G16_SNORM,
		R3G3B2_UNORM,
		R4G4B4_UNORM,
		R5G5B5_UNORM,
		R8G8B8_UNORM,
		R8G8B8_SNORM,
		R10G10B10_UNORM,
		R12G12B12_UNORM,
		R16G16B16_SNORM,
		R2G2B2A2_UNORM,
		R4G4B4A4_UNORM,
		R5G5B5A1_UNORM,
		R8G8B8A8_UNORM,
		R8G8B8A8_SNORM,
		R10G10B10A2_UNORM,
		R10G10B10A2_UINT,
		R12G12B12A12_UNORM,
		R16G16B16A16_UNORM,
		R16G16B16A16_SNORM,
		R8G8B8_SRGB,
		R8G8B8A8_SRGB,
		R16_FLOAT,
		R16G16_FLOAT,
		R16G16B16_FLOAT,
		R16G16B16A16_FLOAT,
		R32_FLOAT,
		R32G32_FLOAT,
		R32G32B32_FLOAT,
		R32G32B32A32_FLOAT,
		R11G11B10_FLOAT,
		R9G9B9_E5,
		R8_SINT,
		R8_UINT,
		R16_SINT,
		R16_UINT,
		R32_SINT,
		R32_UINT,
		R8G8_SINT,
		R8G8_UINT,
		R16G16_SINT,
		R16G16_UINT,
		R32G32_SINT,
		R32G32_UINT,
		R8G8B8_SINT,
		R8G8B8_UINT,
		R16G16B16_SINT,
		R16G16B16_UINT,
		R32G32B32_SINT,
		R32G32B32_UINT,
		R8G8B8A8_SINT,
		R8G8B8A8_UINT,
		R16G16B16A16_SINT,
		R16G16B16A16_UINT,
		R32G32B32A32_SINT,
		R32G32B32A32_UINT,

		// Depth & stencil formats
		D32_FLOAT,
		D32_UNORM,
		D24_UNORM,
		D16_UNORM,
		D32_FLOAT_S8_UINT,
		D24_UNORM_S8_UINT,
		S8_UINT,

		// Compressed formats
		// DXT
		BC1_RGB_UNORM,
		BC1_RGB_SRGB,
		BC1_RGBA_UNORM,
		BC1_RGBA_SRGB,
		BC2_RGBA_UNORM,
		BC2_RGBA_SRGB,
		BC3_RGBA_UNORM,
		BC3_RGBA_SRGB,
		// RGTC
		BC4_R_UNORM,
		BC4_R_SNORM,
		BC5_RG_UNORM,
		BC5_RG_SNORM,
		// BPTC
		BC6H_RGB_UFLOAT,
		BC6H_RGB_SFLOAT,
		BC7_RGBA_UNORM,
		BC7_RGBA_SRGB,

		// TODO: 64-bits-per-component formats
	};
	inline GLint FormatToGL(Format format)
	{
		switch (format)
		{
		case Format::R8_UNORM:           return GL_R8;
		case Format::R8_SNORM:           return GL_R8_SNORM;
		case Format::R16_UNORM:          return GL_R16;
		case Format::R16_SNORM:          return GL_R16_SNORM;
		case Format::R8G8_UNORM:         return GL_RG8;
		case Format::R8G8_SNORM:         return GL_RG8_SNORM;
		case Format::R16G16_UNORM:       return GL_RG16;
		case Format::R16G16_SNORM:       return GL_RG16_SNORM;
		case Format::R3G3B2_UNORM:       return GL_R3_G3_B2;
		case Format::R4G4B4_UNORM:       return GL_RGB4;
		case Format::R5G5B5_UNORM:       return GL_RGB5;
		case Format::R8G8B8_UNORM:       return GL_RGB8;
		case Format::R8G8B8_SNORM:       return GL_RGB8_SNORM;
		case Format::R10G10B10_UNORM:    return GL_RGB10;
		case Format::R12G12B12_UNORM:    return GL_RGB12;
			// GL_RG16?
		case Format::R16G16B16_SNORM:    return GL_RGB16_SNORM;
		case Format::R2G2B2A2_UNORM:     return GL_RGBA2;
		case Format::R4G4B4A4_UNORM:     return GL_RGBA4;
		case Format::R5G5B5A1_UNORM:     return GL_RGB5_A1;
		case Format::R8G8B8A8_UNORM:     return GL_RGBA8;
		case Format::R8G8B8A8_SNORM:     return GL_RGBA8_SNORM;
		case Format::R10G10B10A2_UNORM:  return GL_RGB10_A2;
		case Format::R10G10B10A2_UINT:   return GL_RGB10_A2UI;
		case Format::R12G12B12A12_UNORM: return GL_RGBA12;
		case Format::R16G16B16A16_UNORM: return GL_RGBA16;
		case Format::R16G16B16A16_SNORM: return GL_RGBA16_SNORM;
		case Format::R8G8B8_SRGB:        return GL_SRGB8;
		case Format::R8G8B8A8_SRGB:      return GL_SRGB8_ALPHA8;
		case Format::R16_FLOAT:          return GL_R16F;
		case Format::R16G16_FLOAT:       return GL_RG16F;
		case Format::R16G16B16_FLOAT:    return GL_RGB16F;
		case Format::R16G16B16A16_FLOAT: return GL_RGBA16F;
		case Format::R32_FLOAT:          return GL_R32F;
		case Format::R32G32_FLOAT:       return GL_RG32F;
		case Format::R32G32B32_FLOAT:    return GL_RGB32F;
		case Format::R32G32B32A32_FLOAT: return GL_RGBA32F;
		case Format::R11G11B10_FLOAT:    return GL_R11F_G11F_B10F;
		case Format::R9G9B9_E5:          return GL_RGB9_E5;
		case Format::R8_SINT:            return GL_R8I;
		case Format::R8_UINT:            return GL_R8UI;
		case Format::R16_SINT:           return GL_R16I;
		case Format::R16_UINT:           return GL_R16UI;
		case Format::R32_SINT:           return GL_R32I;
		case Format::R32_UINT:           return GL_R32UI;
		case Format::R8G8_SINT:          return GL_RG8I;
		case Format::R8G8_UINT:          return GL_RG8UI;
		case Format::R16G16_SINT:        return GL_RG16I;
		case Format::R16G16_UINT:        return GL_RG16UI;
		case Format::R32G32_SINT:        return GL_RG32I;
		case Format::R32G32_UINT:        return GL_RG32UI;
		case Format::R8G8B8_SINT:        return GL_RGB8I;
		case Format::R8G8B8_UINT:        return GL_RGB8UI;
		case Format::R16G16B16_SINT:     return GL_RGB16I;
		case Format::R16G16B16_UINT:     return GL_RGB16UI;
		case Format::R32G32B32_SINT:     return GL_RGB32I;
		case Format::R32G32B32_UINT:     return GL_RGB32UI;
		case Format::R8G8B8A8_SINT:      return GL_RGBA8I;
		case Format::R8G8B8A8_UINT:      return GL_RGBA8UI;
		case Format::R16G16B16A16_SINT:  return GL_RGBA16I;
		case Format::R16G16B16A16_UINT:  return GL_RGBA16UI;
		case Format::R32G32B32A32_SINT:  return GL_RGBA32I;
		case Format::R32G32B32A32_UINT:  return GL_RGBA32UI;
		case Format::D32_FLOAT:          return GL_DEPTH_COMPONENT32F;
		case Format::D32_UNORM:          return GL_DEPTH_COMPONENT32;
		case Format::D24_UNORM:          return GL_DEPTH_COMPONENT24;
		case Format::D16_UNORM:          return GL_DEPTH_COMPONENT16;
		case Format::D32_FLOAT_S8_UINT:  return GL_DEPTH32F_STENCIL8;
		case Format::D24_UNORM_S8_UINT:  return GL_DEPTH24_STENCIL8;
		case Format::S8_UINT:            return GL_STENCIL_INDEX8;
		case Format::BC1_RGB_UNORM:      return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		case Format::BC1_RGBA_UNORM:     return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		case Format::BC1_RGB_SRGB:       return GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
		case Format::BC1_RGBA_SRGB:      return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
		case Format::BC2_RGBA_UNORM:     return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		case Format::BC2_RGBA_SRGB:      return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
		case Format::BC3_RGBA_UNORM:     return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		case Format::BC3_RGBA_SRGB:      return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
		case Format::BC4_R_UNORM:        return GL_COMPRESSED_RED_RGTC1;
		case Format::BC4_R_SNORM:        return GL_COMPRESSED_SIGNED_RED_RGTC1;
		case Format::BC5_RG_UNORM:       return GL_COMPRESSED_RG_RGTC2;
		case Format::BC5_RG_SNORM:       return GL_COMPRESSED_SIGNED_RG_RGTC2;
		case Format::BC6H_RGB_UFLOAT:    return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;
		case Format::BC6H_RGB_SFLOAT:    return GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT;
		case Format::BC7_RGBA_UNORM:     return GL_COMPRESSED_RGBA_BPTC_UNORM;
		case Format::BC7_RGBA_SRGB:      return GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;
		default: assert(0);              return 0;
		}
	}
	inline bool IsBlockCompressedFormat(Format format)
	{
		switch (format)
		{
		case Format::BC1_RGB_UNORM:
		case Format::BC1_RGBA_UNORM:
		case Format::BC1_RGB_SRGB:
		case Format::BC1_RGBA_SRGB:
		case Format::BC2_RGBA_UNORM:
		case Format::BC2_RGBA_SRGB:
		case Format::BC3_RGBA_UNORM:
		case Format::BC3_RGBA_SRGB:
		case Format::BC4_R_UNORM:
		case Format::BC4_R_SNORM:
		case Format::BC5_RG_UNORM:
		case Format::BC5_RG_SNORM:
		case Format::BC6H_RGB_UFLOAT:
		case Format::BC6H_RGB_SFLOAT:
		case Format::BC7_RGBA_UNORM:
		case Format::BC7_RGBA_SRGB:
			return true;
		default: return false;
		}
	}
	inline GLenum FormatToTypeGL(Format format)
	{
		switch (format)
		{
		case Format::R8_UNORM:
		case Format::R8G8_UNORM:
		case Format::R8G8B8_UNORM:
		case Format::R8G8B8A8_UNORM:
		case Format::R8_UINT:
		case Format::R8G8_UINT:
		case Format::R8G8B8_UINT:
		case Format::R8G8B8A8_UINT:
		case Format::R8G8B8A8_SRGB:
		case Format::R8G8B8_SRGB:
			return GL_UNSIGNED_BYTE;
		case Format::R8_SNORM:
		case Format::R8G8_SNORM:
		case Format::R8G8B8_SNORM:
		case Format::R8G8B8A8_SNORM:
		case Format::R8_SINT:
		case Format::R8G8_SINT:
		case Format::R8G8B8_SINT:
		case Format::R8G8B8A8_SINT:
			return GL_BYTE;
		case Format::R16_UNORM:
		case Format::R16G16_UNORM:
		case Format::R16G16B16A16_UNORM:
		case Format::R16_UINT:
		case Format::R16G16_UINT:
		case Format::R16G16B16_UINT:
		case Format::R16G16B16A16_UINT:
			return GL_UNSIGNED_SHORT;
		case Format::R16_SNORM:
		case Format::R16G16_SNORM:
		case Format::R16G16B16_SNORM:
		case Format::R16G16B16A16_SNORM:
		case Format::R16_SINT:
		case Format::R16G16_SINT:
		case Format::R16G16B16_SINT:
		case Format::R16G16B16A16_SINT:
			return GL_SHORT;
		case Format::R16_FLOAT:
		case Format::R16G16_FLOAT:
		case Format::R16G16B16_FLOAT:
		case Format::R16G16B16A16_FLOAT:
			return GL_HALF_FLOAT;
		case Format::R32_FLOAT:
		case Format::R32G32_FLOAT:
		case Format::R32G32B32_FLOAT:
		case Format::R32G32B32A32_FLOAT:
			return GL_FLOAT;
		case Format::R32_SINT:
		case Format::R32G32_SINT:
		case Format::R32G32B32_SINT:
		case Format::R32G32B32A32_SINT:
			return GL_INT;
		case Format::R32_UINT:
		case Format::R32G32_UINT:
		case Format::R32G32B32_UINT:
		case Format::R32G32B32A32_UINT:
			return GL_UNSIGNED_INT;
		default: assert(0); return 0;
		}
	}
	inline GLint FormatToSizeGL(Format format)
	{
		switch (format)
		{
		case Format::R8_UNORM:
		case Format::R8_SNORM:
		case Format::R16_UNORM:
		case Format::R16_SNORM:
		case Format::R16_FLOAT:
		case Format::R32_FLOAT:
		case Format::R8_SINT:
		case Format::R16_SINT:
		case Format::R32_SINT:
		case Format::R8_UINT:
		case Format::R16_UINT:
		case Format::R32_UINT:
			return 1;
		case Format::R8G8_UNORM:
		case Format::R8G8_SNORM:
		case Format::R16G16_FLOAT:
		case Format::R16G16_UNORM:
		case Format::R16G16_SNORM:
		case Format::R32G32_FLOAT:
		case Format::R8G8_SINT:
		case Format::R16G16_SINT:
		case Format::R32G32_SINT:
		case Format::R8G8_UINT:
		case Format::R16G16_UINT:
		case Format::R32G32_UINT:
			return 2;
		case Format::R8G8B8_UNORM:
		case Format::R8G8B8_SNORM:
		case Format::R16G16B16_SNORM:
		case Format::R16G16B16_FLOAT:
		case Format::R32G32B32_FLOAT:
		case Format::R8G8B8_SINT:
		case Format::R16G16B16_SINT:
		case Format::R32G32B32_SINT:
		case Format::R8G8B8_UINT:
		case Format::R16G16B16_UINT:
		case Format::R32G32B32_UINT:
			return 3;
		case Format::R8G8B8A8_UNORM:
		case Format::R8G8B8A8_SNORM:
		case Format::R16G16B16A16_UNORM:
		case Format::R16G16B16A16_SNORM:
		case Format::R16G16B16A16_FLOAT:
		case Format::R32G32B32A32_FLOAT:
		case Format::R8G8B8A8_SINT:
		case Format::R16G16B16A16_SINT:
		case Format::R32G32B32A32_SINT:
		case Format::R10G10B10A2_UINT:
		case Format::R8G8B8A8_UINT:
		case Format::R16G16B16A16_UINT:
		case Format::R32G32B32A32_UINT:
			return 4;
		default: assert(0); return 0;
		}
	}
	inline GLboolean IsFormatNormalizedGL(Format format)
	{
		switch (format)
		{
		case Format::R8_UNORM:
		case Format::R8_SNORM:
		case Format::R16_UNORM:
		case Format::R16_SNORM:
		case Format::R8G8_UNORM:
		case Format::R8G8_SNORM:
		case Format::R16G16_UNORM:
		case Format::R16G16_SNORM:
		case Format::R8G8B8_UNORM:
		case Format::R8G8B8_SNORM:
		case Format::R16G16B16_SNORM:
		case Format::R8G8B8A8_UNORM:
		case Format::R8G8B8A8_SNORM:
		case Format::R16G16B16A16_UNORM:
		case Format::R16G16B16A16_SNORM:
			return GL_TRUE;
		case Format::R16_FLOAT:
		case Format::R32_FLOAT:
		case Format::R8_SINT:
		case Format::R16_SINT:
		case Format::R32_SINT:
		case Format::R8_UINT:
		case Format::R16_UINT:
		case Format::R32_UINT:
		case Format::R16G16_FLOAT:
		case Format::R32G32_FLOAT:
		case Format::R8G8_SINT:
		case Format::R16G16_SINT:
		case Format::R32G32_SINT:
		case Format::R8G8_UINT:
		case Format::R16G16_UINT:
		case Format::R32G32_UINT:
		case Format::R16G16B16_FLOAT:
		case Format::R32G32B32_FLOAT:
		case Format::R8G8B8_SINT:
		case Format::R16G16B16_SINT:
		case Format::R32G32B32_SINT:
		case Format::R8G8B8_UINT:
		case Format::R16G16B16_UINT:
		case Format::R32G32B32_UINT:
		case Format::R16G16B16A16_FLOAT:
		case Format::R32G32B32A32_FLOAT:
		case Format::R8G8B8A8_SINT:
		case Format::R16G16B16A16_SINT:
		case Format::R32G32B32A32_SINT:
		case Format::R10G10B10A2_UINT:
		case Format::R8G8B8A8_UINT:
		case Format::R16G16B16A16_UINT:
		case Format::R32G32B32A32_UINT:
			return GL_FALSE;
		default: assert(0); return 0;
		}
	}
	inline GlFormatClass FormatToFormatClass(Format format)
	{
		switch (format)
		{
		case Format::R8_UNORM:
		case Format::R8_SNORM:
		case Format::R16_UNORM:
		case Format::R16_SNORM:
		case Format::R8G8_UNORM:
		case Format::R8G8_SNORM:
		case Format::R16G16_UNORM:
		case Format::R16G16_SNORM:
		case Format::R8G8B8_UNORM:
		case Format::R8G8B8_SNORM:
		case Format::R16G16B16_SNORM:
		case Format::R8G8B8A8_UNORM:
		case Format::R8G8B8A8_SNORM:
		case Format::R16G16B16A16_UNORM:
		case Format::R16G16B16A16_SNORM:
		case Format::R16_FLOAT:
		case Format::R16G16_FLOAT:
		case Format::R16G16B16_FLOAT:
		case Format::R16G16B16A16_FLOAT:
		case Format::R32_FLOAT:
		case Format::R32G32_FLOAT:
		case Format::R32G32B32_FLOAT:
		case Format::R32G32B32A32_FLOAT:
			return GlFormatClass::Float;
		case Format::R8_SINT:
		case Format::R16_SINT:
		case Format::R32_SINT:
		case Format::R8G8_SINT:
		case Format::R16G16_SINT:
		case Format::R32G32_SINT:
		case Format::R8G8B8_SINT:
		case Format::R16G16B16_SINT:
		case Format::R32G32B32_SINT:
		case Format::R8G8B8A8_SINT:
		case Format::R16G16B16A16_SINT:
		case Format::R32G32B32A32_SINT:
		case Format::R10G10B10A2_UINT:
		case Format::R8_UINT:
		case Format::R16_UINT:
		case Format::R32_UINT:
		case Format::R8G8_UINT:
		case Format::R16G16_UINT:
		case Format::R32G32_UINT:
		case Format::R8G8B8_UINT:
		case Format::R16G16B16_UINT:
		case Format::R32G32B32_UINT:
		case Format::R8G8B8A8_UINT:
		case Format::R16G16B16A16_UINT:
		case Format::R32G32B32A32_UINT:
			return GlFormatClass::Int;
		default: assert(0); return GlFormatClass::Long;
		}
	}
	inline bool IsValidImageFormat(Format format)
	{
		switch (format)
		{
		case Format::R32G32B32A32_FLOAT:
		case Format::R16G16B16A16_FLOAT:
		case Format::R32G32_FLOAT:
		case Format::R16G16_FLOAT:
		case Format::R11G11B10_FLOAT:
		case Format::R32_FLOAT:
		case Format::R16_FLOAT:
		case Format::R32G32B32A32_UINT:
		case Format::R16G16B16A16_UINT:
		case Format::R10G10B10A2_UINT:
		case Format::R8G8B8A8_UINT:
		case Format::R32G32_UINT:
		case Format::R16G16_UINT:
		case Format::R8G8_UINT:
		case Format::R32_UINT:
		case Format::R16_UINT:
		case Format::R8_UINT:
		case Format::R32G32B32_SINT:
		case Format::R16G16B16A16_SINT:
		case Format::R8G8B8A8_SINT:
		case Format::R32G32_SINT:
		case Format::R16G16_SINT:
		case Format::R8G8_SINT:
		case Format::R32_SINT:
		case Format::R16_SINT:
		case Format::R8_SINT:
		case Format::R16G16B16A16_UNORM:
		case Format::R10G10B10A2_UNORM:
		case Format::R8G8B8A8_UNORM:
		case Format::R16G16_UNORM:
		case Format::R8G8_UNORM:
		case Format::R16_UNORM:
		case Format::R8_UNORM:
		case Format::R16G16B16A16_SNORM:
		case Format::R8G8B8A8_SNORM:
		case Format::R16G16_SNORM:
		case Format::R8G8_SNORM:
		case Format::R16_SNORM:
		case Format::R8_SNORM: return true;
		default: return false;
		}
	}
	inline bool IsDepthFormat(Format format)
	{
		switch (format)
		{
		case Format::D32_FLOAT:
		case Format::D32_UNORM:
		case Format::D24_UNORM:
		case Format::D16_UNORM:
		case Format::D32_FLOAT_S8_UINT:
		case Format::D24_UNORM_S8_UINT: return true;
		default: return false;
		}
	}
	inline bool IsStencilFormat(Format format)
	{
		switch (format)
		{
		case Format::D32_FLOAT_S8_UINT:
		case Format::D24_UNORM_S8_UINT: return true;
		default: return false;
		}
	}
	inline bool IsColorFormat(Format format)
	{
		return !IsDepthFormat(format) && !IsStencilFormat(format);
	}

	enum class BufferStorageFlag : uint32_t
	{
		NONE = 0,
		// Allows the user to update the buffer's contents with UpdateData
		DYNAMIC_STORAGE = 1 << 0,
		// Hints to the implementation to place the buffer storage in host memory
		CLIENT_STORAGE = 1 << 1,
		// Maps the buffer (persistently and coherently) upon creation
		MAP_MEMORY = 1 << 2,
	};
	SE_DECLARE_FLAG_TYPE(BufferStorageFlags, BufferStorageFlag, uint32_t)

	inline GLbitfield BufferStorageFlagsToGL(BufferStorageFlags flags)
	{
		GLbitfield ret = 0;
		ret |= flags & BufferStorageFlag::DYNAMIC_STORAGE ? GL_DYNAMIC_STORAGE_BIT : 0;
		ret |= flags & BufferStorageFlag::CLIENT_STORAGE ? GL_CLIENT_STORAGE_BIT : 0;

		// As far as I can tell, there is no perf hit to having both MAP_WRITE and MAP_READ all the time.
		// Additionally, desktop platforms (the ones we care about) do not have incoherent host-visible device heaps, so we can safely include that flag all the time.
		// https://gpuopen.com/learn/get-the-most-out-of-smart-access-memory/
		// https://basnieuwenhuizen.nl/the-catastrophe-of-reading-from-vram/
		// https://asawicki.info/news_1740_vulkan_memory_types_on_pc_and_how_to_use_them
		constexpr GLenum memMapFlags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		ret |= flags & BufferStorageFlag::MAP_MEMORY ? memMapFlags : 0;
		return ret;
	}

	enum class ImageType : uint8_t
	{
		Tex1D,
		Tex2D,
		Tex3D,
		Tex1DArray,
		Tex2DArray,
		TexCubemap,
		TexCubemapArray,
		Tex2DMultisample,
		Tex2DMultisampleArray,
	};
	inline GLint EnumToGL(ImageType imageType)
	{
		switch (imageType)
		{
		case ImageType::Tex1D:                 return GL_TEXTURE_1D;
		case ImageType::Tex2D:                 return GL_TEXTURE_2D;
		case ImageType::Tex3D:                 return GL_TEXTURE_3D;
		case ImageType::Tex1DArray:            return GL_TEXTURE_1D_ARRAY;
		case ImageType::Tex2DArray:            return GL_TEXTURE_2D_ARRAY;
		case ImageType::TexCubemap:            return GL_TEXTURE_CUBE_MAP;
		case ImageType::TexCubemapArray:       return GL_TEXTURE_CUBE_MAP_ARRAY;
		case ImageType::Tex2DMultisample:      return GL_TEXTURE_2D_MULTISAMPLE;
		case ImageType::Tex2DMultisampleArray: return GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
		default: assert(0);                    return 0;
		}
	}
	inline int ImageTypeToDimension(ImageType imageType)
	{
		switch (imageType)
		{
		case ImageType::Tex1D:
			return 1;
		case ImageType::Tex2D:
		case ImageType::Tex2DMultisample:
		case ImageType::Tex1DArray:
			return 2;
		case ImageType::Tex3D:
		case ImageType::Tex2DArray:
		case ImageType::TexCubemap:
		case ImageType::TexCubemapArray:
		case ImageType::Tex2DMultisampleArray:
			return 3;
		default: assert(0); return 0;
		}
	}

	enum class UploadFormat : uint8_t
	{
		UNDEFINED,
		R,
		RG,
		RGB,
		BGR,
		RGBA,
		BGRA,
		R_INTEGER,
		RG_INTEGER,
		RGB_INTEGER,
		BGR_INTEGER,
		RGBA_INTEGER,
		BGRA_INTEGER,
		DEPTH_COMPONENT,
		STENCIL_INDEX,
		DEPTH_STENCIL,

		// @brief For CopyTextureToBuffer and CopyBufferToTexture
		INFER_FORMAT,
	};
	inline GLint EnumToGL(UploadFormat uploadFormat)
	{
		switch (uploadFormat)
		{
		case UploadFormat::R:               return GL_RED;
		case UploadFormat::RG:              return GL_RG;
		case UploadFormat::RGB:             return GL_RGB;
		case UploadFormat::BGR:             return GL_BGR;
		case UploadFormat::RGBA:            return GL_RGBA;
		case UploadFormat::BGRA:            return GL_BGRA;
		case UploadFormat::R_INTEGER:       return GL_RED_INTEGER;
		case UploadFormat::RG_INTEGER:      return GL_RG_INTEGER;
		case UploadFormat::RGB_INTEGER:     return GL_RGB_INTEGER;
		case UploadFormat::BGR_INTEGER:     return GL_BGR_INTEGER;
		case UploadFormat::RGBA_INTEGER:    return GL_RGBA_INTEGER;
		case UploadFormat::BGRA_INTEGER:    return GL_BGRA_INTEGER;
		case UploadFormat::DEPTH_COMPONENT: return GL_DEPTH_COMPONENT;
		case UploadFormat::STENCIL_INDEX:   return GL_STENCIL_INDEX;
		case UploadFormat::DEPTH_STENCIL:   return GL_DEPTH_STENCIL;
		default: assert(0);                 return 0;
		}
	}
	inline UploadFormat FormatToUploadFormat(Format format)
	{
		switch (format)
		{
		case Format::R8_UNORM:
		case Format::R8_SNORM:
		case Format::R16_UNORM:
		case Format::R16_SNORM:
		case Format::R16_FLOAT:
		case Format::R32_FLOAT:
			return UploadFormat::R;
		case Format::R8_SINT:
		case Format::R8_UINT:
		case Format::R16_SINT:
		case Format::R16_UINT:
		case Format::R32_SINT:
		case Format::R32_UINT:
			return UploadFormat::R_INTEGER;
		case Format::R8G8_UNORM:
		case Format::R8G8_SNORM:
		case Format::R16G16_UNORM:
		case Format::R16G16_SNORM:
		case Format::R16G16_FLOAT:
		case Format::R32G32_FLOAT:
			return UploadFormat::RG;
		case Format::R8G8_SINT:
		case Format::R8G8_UINT:
		case Format::R16G16_SINT:
		case Format::R16G16_UINT:
		case Format::R32G32_SINT:
		case Format::R32G32_UINT:
			return UploadFormat::RG_INTEGER;
		case Format::R3G3B2_UNORM:
		case Format::R4G4B4_UNORM:
		case Format::R5G5B5_UNORM:
		case Format::R8G8B8_UNORM:
		case Format::R8G8B8_SNORM:
		case Format::R10G10B10_UNORM:
		case Format::R12G12B12_UNORM:
		case Format::R16G16B16_SNORM:
		case Format::R8G8B8_SRGB:
		case Format::R16G16B16_FLOAT:
		case Format::R9G9B9_E5:
		case Format::R32G32B32_FLOAT:
		case Format::R11G11B10_FLOAT:
			return UploadFormat::RGB;
		case Format::R8G8B8_SINT:
		case Format::R8G8B8_UINT:
		case Format::R16G16B16_SINT:
		case Format::R16G16B16_UINT:
		case Format::R32G32B32_SINT:
		case Format::R32G32B32_UINT:
			return UploadFormat::RGB_INTEGER;
		case Format::R2G2B2A2_UNORM:
		case Format::R4G4B4A4_UNORM:
		case Format::R5G5B5A1_UNORM:
		case Format::R8G8B8A8_UNORM:
		case Format::R8G8B8A8_SNORM:
		case Format::R10G10B10A2_UNORM:
		case Format::R12G12B12A12_UNORM:
		case Format::R16G16B16A16_UNORM:
		case Format::R16G16B16A16_SNORM:
		case Format::R8G8B8A8_SRGB:
		case Format::R16G16B16A16_FLOAT:
		case Format::R32G32B32A32_FLOAT:
			return UploadFormat::RGBA;
		case Format::R10G10B10A2_UINT:
		case Format::R8G8B8A8_SINT:
		case Format::R8G8B8A8_UINT:
		case Format::R16G16B16A16_SINT:
		case Format::R16G16B16A16_UINT:
		case Format::R32G32B32A32_SINT:
		case Format::R32G32B32A32_UINT:
			return UploadFormat::RGBA_INTEGER;
		case Format::D32_FLOAT:
		case Format::D32_UNORM:
		case Format::D24_UNORM:
		case Format::D16_UNORM:
			return UploadFormat::DEPTH_COMPONENT;
		case Format::D32_FLOAT_S8_UINT:
		case Format::D24_UNORM_S8_UINT:
			return UploadFormat::DEPTH_STENCIL;
		case Format::S8_UINT:
			return UploadFormat::STENCIL_INDEX;
		default: assert(0); return {};
		}
	}

	enum class UploadType : uint8_t
	{
		UNDEFINED,
		UBYTE,
		SBYTE,
		USHORT,
		SSHORT,
		UINT,
		SINT,
		FLOAT,
		UBYTE_3_3_2,
		UBYTE_2_3_3_REV,
		USHORT_5_6_5,
		USHORT_5_6_5_REV,
		USHORT_4_4_4_4,
		USHORT_4_4_4_4_REV,
		USHORT_5_5_5_1,
		USHORT_1_5_5_5_REV,
		UINT_8_8_8_8,
		UINT_8_8_8_8_REV,
		UINT_10_10_10_2,
		UINT_2_10_10_10_REV,

		// @brief For CopyTextureToBuffer and CopyBufferToTexture
		INFER_TYPE,
	};
	inline GLint EnumToGL(UploadType uploadType)
	{
		switch (uploadType)
		{
		case UploadType::UBYTE:               return GL_UNSIGNED_BYTE;
		case UploadType::SBYTE:               return GL_BYTE;
		case UploadType::USHORT:              return GL_UNSIGNED_SHORT;
		case UploadType::SSHORT:              return GL_SHORT;
		case UploadType::UINT:                return GL_UNSIGNED_INT;
		case UploadType::SINT:                return GL_INT;
		case UploadType::FLOAT:               return GL_FLOAT;
		case UploadType::UBYTE_3_3_2:         return GL_UNSIGNED_BYTE_3_3_2;
		case UploadType::UBYTE_2_3_3_REV:     return GL_UNSIGNED_BYTE_2_3_3_REV;
		case UploadType::USHORT_5_6_5:        return GL_UNSIGNED_SHORT_5_6_5;
		case UploadType::USHORT_5_6_5_REV:    return GL_UNSIGNED_SHORT_5_6_5_REV;
		case UploadType::USHORT_4_4_4_4:      return GL_UNSIGNED_SHORT_4_4_4_4;
		case UploadType::USHORT_4_4_4_4_REV:  return GL_UNSIGNED_SHORT_4_4_4_4_REV;
		case UploadType::USHORT_5_5_5_1:      return GL_UNSIGNED_SHORT_5_5_5_1;
		case UploadType::USHORT_1_5_5_5_REV:  return GL_UNSIGNED_SHORT_1_5_5_5_REV;
		case UploadType::UINT_8_8_8_8:        return GL_UNSIGNED_INT_8_8_8_8;
		case UploadType::UINT_8_8_8_8_REV:    return GL_UNSIGNED_INT_8_8_8_8_REV;
		case UploadType::UINT_10_10_10_2:     return GL_UNSIGNED_INT_10_10_10_2;
		case UploadType::UINT_2_10_10_10_REV: return GL_UNSIGNED_INT_2_10_10_10_REV;
		default: assert(0);                   return 0;
		}
	}

	enum class AspectMaskBit : uint32_t
	{
		COLOR_BUFFER_BIT = 1 << 0,
		DEPTH_BUFFER_BIT = 1 << 1,
		STENCIL_BUFFER_BIT = 1 << 2,
	};
	SE_DECLARE_FLAG_TYPE(AspectMask, AspectMaskBit, uint32_t)
	inline GLbitfield AspectMaskToGL(AspectMask bits)
	{
		GLbitfield ret = 0;
		ret |= bits & AspectMaskBit::COLOR_BUFFER_BIT ? GL_COLOR_BUFFER_BIT : 0;
		ret |= bits & AspectMaskBit::DEPTH_BUFFER_BIT ? GL_DEPTH_BUFFER_BIT : 0;
		ret |= bits & AspectMaskBit::STENCIL_BUFFER_BIT ? GL_STENCIL_BUFFER_BIT : 0;
		return ret;
	}

	enum class IndexType : uint8_t
	{
		UNSIGNED_BYTE,
		UNSIGNED_SHORT,
		UNSIGNED_INT,
	};
	inline GLenum EnumToGL(IndexType type)
	{
		switch (type)
		{
		case IndexType::UNSIGNED_BYTE: return GL_UNSIGNED_BYTE;
		case IndexType::UNSIGNED_SHORT: return GL_UNSIGNED_SHORT;
		case IndexType::UNSIGNED_INT: return GL_UNSIGNED_INT;
		default: assert(0); return 0;
		}
	}
	inline size_t GetIndexSize(IndexType indexType)
	{
		switch (indexType)
		{
		case IndexType::UNSIGNED_BYTE: return 1;
		case IndexType::UNSIGNED_SHORT: return 2;
		case IndexType::UNSIGNED_INT: return 4;
		default: assert(0); return 0;
		}
	}

	enum class MemoryBarrierBit : uint32_t
	{
		NONE = 0,
		VERTEX_BUFFER_BIT = 1 << 0,  // GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT
		INDEX_BUFFER_BIT = 1 << 1,  // GL_ELEMENT_ARRAY_BARRIER_BIT
		UNIFORM_BUFFER_BIT = 1 << 2,  // GL_UNIFORM_BARRIER_BIT
		TEXTURE_FETCH_BIT = 1 << 3,  // GL_TEXTURE_FETCH_BARRIER_BIT
		IMAGE_ACCESS_BIT = 1 << 4,  // GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
		COMMAND_BUFFER_BIT = 1 << 5,  // GL_COMMAND_BARRIER_BIT
		TEXTURE_UPDATE_BIT = 1 << 6,  // GL_TEXTURE_UPDATE_BARRIER_BIT
		BUFFER_UPDATE_BIT = 1 << 7,  // GL_BUFFER_UPDATE_BARRIER_BIT
		MAPPED_BUFFER_BIT = 1 << 8,  // GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT
		FRAMEBUFFER_BIT = 1 << 9,  // GL_FRAMEBUFFER_BARRIER_BIT
		SHADER_STORAGE_BIT = 1 << 10, // GL_SHADER_STORAGE_BARRIER_BIT
		QUERY_COUNTER_BIT = 1 << 11, // GL_QUERY_BUFFER_BARRIER_BIT
		ALL_BITS = static_cast<uint32_t>(-1),
		// TODO: add more bits as necessary
	};
	SE_DECLARE_FLAG_TYPE(MemoryBarrierBits, MemoryBarrierBit, uint32_t)
	inline GLbitfield BarrierBitsToGL(MemoryBarrierBits bits)
	{
		GLbitfield ret = 0;
		ret |= bits & MemoryBarrierBit::VERTEX_BUFFER_BIT ? GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT : 0;
		ret |= bits & MemoryBarrierBit::INDEX_BUFFER_BIT ? GL_ELEMENT_ARRAY_BARRIER_BIT : 0;
		ret |= bits & MemoryBarrierBit::UNIFORM_BUFFER_BIT ? GL_UNIFORM_BARRIER_BIT : 0;
		ret |= bits & MemoryBarrierBit::TEXTURE_FETCH_BIT ? GL_TEXTURE_FETCH_BARRIER_BIT : 0;
		ret |= bits & MemoryBarrierBit::IMAGE_ACCESS_BIT ? GL_SHADER_IMAGE_ACCESS_BARRIER_BIT : 0;
		ret |= bits & MemoryBarrierBit::COMMAND_BUFFER_BIT ? GL_COMMAND_BARRIER_BIT : 0;
		ret |= bits & MemoryBarrierBit::TEXTURE_UPDATE_BIT ? GL_TEXTURE_UPDATE_BARRIER_BIT : 0;
		ret |= bits & MemoryBarrierBit::BUFFER_UPDATE_BIT ? GL_BUFFER_UPDATE_BARRIER_BIT : 0;
		ret |= bits & MemoryBarrierBit::MAPPED_BUFFER_BIT ? GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT : 0;
		ret |= bits & MemoryBarrierBit::FRAMEBUFFER_BIT ? GL_FRAMEBUFFER_BARRIER_BIT : 0;
		ret |= bits & MemoryBarrierBit::SHADER_STORAGE_BIT ? GL_SHADER_STORAGE_BARRIER_BIT : 0;
		ret |= bits & MemoryBarrierBit::QUERY_COUNTER_BIT ? GL_QUERY_BUFFER_BARRIER_BIT : 0;
		return ret;
	}

	enum class ComponentSwizzle : uint8_t
	{
		ZERO,
		ONE,
		R,
		G,
		B,
		A
	};
	inline GLint EnumToGL(ComponentSwizzle swizzle)
	{
		switch (swizzle)
		{
		case ComponentSwizzle::ZERO: return GL_ZERO;
		case ComponentSwizzle::ONE: return GL_ONE;
		case ComponentSwizzle::R: return GL_RED;
		case ComponentSwizzle::G: return GL_GREEN;
		case ComponentSwizzle::B: return GL_BLUE;
		case ComponentSwizzle::A: return GL_ALPHA;
		default: assert(0); return 0;
		}
	}

	enum class ColorComponentFlag : uint32_t
	{
		NONE,
		R_BIT = 0b0001,
		G_BIT = 0b0010,
		B_BIT = 0b0100,
		A_BIT = 0b1000,
		RGBA_BITS = 0b1111,
	};
	SE_DECLARE_FLAG_TYPE(ColorComponentFlags, ColorComponentFlag, uint32_t)

	enum class ClipDepthRange : uint8_t
	{
		NEGATIVE_ONE_TO_ONE, // OpenGL default
		ZERO_TO_ONE         // D3D and Vulkan
	};
	inline GLenum EnumToGL(ClipDepthRange depthRange)
	{
		if (depthRange == ClipDepthRange::NEGATIVE_ONE_TO_ONE)
			return GL_NEGATIVE_ONE_TO_ONE;
		return GL_ZERO_TO_ONE;
	}

	enum class PrimitiveTopology : uint8_t
	{
		POINT_LIST,
		LINE_LIST,
		LINE_STRIP,
		TRIANGLE_LIST,
		TRIANGLE_STRIP,
		TRIANGLE_FAN,

		/// @note Available only in pipelines with tessellation shaders
		PATCH_LIST,
	};
	inline GLenum EnumToGL(PrimitiveTopology topology)
	{
		switch (topology)
		{
		case PrimitiveTopology::POINT_LIST: return GL_POINTS;
		case PrimitiveTopology::LINE_LIST: return GL_LINES;
		case PrimitiveTopology::LINE_STRIP: return GL_LINE_STRIP;
		case PrimitiveTopology::TRIANGLE_LIST: return GL_TRIANGLES;
		case PrimitiveTopology::TRIANGLE_STRIP: return GL_TRIANGLE_STRIP;
		case PrimitiveTopology::TRIANGLE_FAN: return GL_TRIANGLE_FAN;
		case PrimitiveTopology::PATCH_LIST: return GL_PATCHES;
		default: assert(0); return 0;
		}
	}

	struct TextureCreateInfo final
	{
		ImageType imageType{};
		Format format{};
		Extent3D extent{};
		uint32_t mipLevels{ 0 };
		uint32_t arrayLayers{ 0 };
		SampleCount sampleCount{};

		bool operator==(const TextureCreateInfo&) const noexcept = default;
	};

	struct ComponentMapping final
	{
		ComponentSwizzle r = ComponentSwizzle::R;
		ComponentSwizzle g = ComponentSwizzle::G;
		ComponentSwizzle b = ComponentSwizzle::B;
		ComponentSwizzle a = ComponentSwizzle::A;
	};

	struct TextureViewCreateInfo final
	{
		/// @note Must be an image type compatible with the base texture as defined by table 8.21 in the OpenGL spec
		ImageType viewType{};
		/// @note Must be a format compatible with the base texture as defined by table 8.22 in the OpenGL spec
		Format format{};
		ComponentMapping components{};
		uint32_t minLevel{ 0 };
		uint32_t numLevels{ 0 };
		uint32_t minLayer{ 0 };
		uint32_t numLayers{ 0 };
	};

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

} // namespace gl4