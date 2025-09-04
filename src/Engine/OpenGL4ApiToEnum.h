#pragma once

#include "OpenGL4Core.h"

namespace gl::detail
{
	inline GLenum EnumToGL(CompareOp op) noexcept
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
		default: std::unreachable();
		}
	}

	inline GLenum EnumToGL(StencilOp op) noexcept
	{
		switch (op)
		{
		case StencilOp::Keep:              return GL_KEEP;
		case StencilOp::Zero:              return GL_ZERO;
		case StencilOp::Replace:           return GL_REPLACE;
		case StencilOp::IncrementAndClamp: return GL_INCR;
		case StencilOp::DecrementAndClamp: return GL_DECR;
		case StencilOp::Invert:            return GL_INVERT;
		case StencilOp::IncrementAndWrap:  return GL_INCR_WRAP;
		case StencilOp::DecrementAndWrap:  return GL_DECR_WRAP;
		default: std::unreachable();
		}
	}

	inline GLenum EnumToGL(PolygonMode mode) noexcept
	{
		switch (mode)
		{
		case PolygonMode::Fill:  return GL_FILL;
		case PolygonMode::Line:  return GL_LINE;
		case PolygonMode::Point: return GL_POINT;
		default: std::unreachable();
		}
	}

	inline GLenum EnumToGL(CullMode mode) noexcept
	{
		switch (mode)
		{
		case CullMode::None:         return 0;
		case CullMode::Front:        return GL_FRONT;
		case CullMode::Back:         return GL_BACK;
		case CullMode::FrontAndBack: return GL_FRONT_AND_BACK;
		default: std::unreachable();
		}
	}

	inline GLenum EnumToGL(FrontFace face) noexcept
	{
		switch (face)
		{
		case FrontFace::Clockwise:        return GL_CW;
		case FrontFace::CounterClockwise: return GL_CCW;
		default: std::unreachable();
		}
	}

	inline GLenum EnumToGL(BlendFactor factor) noexcept
	{
		switch (factor)
		{
		case BlendFactor::Zero:                  return GL_ZERO;
		case BlendFactor::One:                   return GL_ONE;
		case BlendFactor::SrcColor:              return GL_SRC_COLOR;
		case BlendFactor::OneMinusSrcColor:      return GL_ONE_MINUS_SRC_COLOR;
		case BlendFactor::DstColor:              return GL_DST_COLOR;
		case BlendFactor::OneMinusDstColor:      return GL_ONE_MINUS_DST_COLOR;
		case BlendFactor::SrcAlpha:              return GL_SRC_ALPHA;
		case BlendFactor::OneMinusSrcAlpha:      return GL_ONE_MINUS_SRC_ALPHA;
		case BlendFactor::DstAlpha:              return GL_DST_ALPHA;
		case BlendFactor::OneMinusDstAlpha:      return GL_ONE_MINUS_DST_ALPHA;
		case BlendFactor::ConstantColor:         return GL_CONSTANT_COLOR;
		case BlendFactor::OneMinusConstantColor: return GL_ONE_MINUS_CONSTANT_COLOR;
		case BlendFactor::ConstantAlpha:         return GL_CONSTANT_ALPHA;
		case BlendFactor::OneMinusConstantAlpha: return GL_ONE_MINUS_CONSTANT_ALPHA;
		case BlendFactor::SrcAlphaSaturate:      return GL_SRC_ALPHA_SATURATE;
		case BlendFactor::Src1Color:             return GL_SRC1_COLOR;
		case BlendFactor::OneMinusSrc1Color:     return GL_ONE_MINUS_SRC1_COLOR;
		case BlendFactor::Src1Alpha:             return GL_SRC1_ALPHA;
		case BlendFactor::OneMinusSrc1Alpha:     return GL_ONE_MINUS_SRC1_ALPHA;
		default: std::unreachable();
		}
	}

	inline GLenum EnumToGL(BlendOp op) noexcept
	{
		switch (op)
		{
		case BlendOp::Add:             return GL_FUNC_ADD;
		case BlendOp::Subtract:        return GL_FUNC_SUBTRACT;
		case BlendOp::ReverseSubtract: return GL_FUNC_REVERSE_SUBTRACT;
		case BlendOp::Min:             return GL_MIN;
		case BlendOp::Max:             return GL_MAX;
		default: std::unreachable();
		}
	}

	inline GLenum EnumToGL(LogicOp op) noexcept
	{
		switch (op)
		{
		case LogicOp::Clear:        return GL_CLEAR;
		case LogicOp::Set:          return GL_SET;
		case LogicOp::Copy:         return GL_COPY;
		case LogicOp::CopyInverted: return GL_COPY_INVERTED;
		case LogicOp::NoOp:         return GL_NOOP;
		case LogicOp::Invert:       return GL_INVERT;
		case LogicOp::And:          return GL_AND;
		case LogicOp::Nand:         return GL_NAND;
		case LogicOp::Or:           return GL_OR;
		case LogicOp::Nor:          return GL_NOR;
		case LogicOp::Xor:          return GL_XOR;
		case LogicOp::Equivalent:   return GL_EQUIV;
		case LogicOp::AndReverse:   return GL_AND_REVERSE;
		case LogicOp::OrReverse:    return GL_OR_REVERSE;
		case LogicOp::AndInverted:  return GL_AND_INVERTED;
		case LogicOp::OrInverted:   return GL_OR_INVERTED;
		default: std::unreachable();
		}
	}

	inline GLbitfield AspectMaskToGL(AspectMask bits) noexcept
	{
		GLbitfield ret = 0;
		ret |= bits & AspectMaskBit::ColorBufferBit ? GL_COLOR_BUFFER_BIT : 0;
		ret |= bits & AspectMaskBit::DepthBufferBit ? GL_DEPTH_BUFFER_BIT : 0;
		ret |= bits & AspectMaskBit::StencilBufferBit ? GL_STENCIL_BUFFER_BIT : 0;
		return ret;
	}

	inline GLenum EnumToGL(MagFilter filter) noexcept
	{
		switch (filter)
		{
		case MagFilter::Nearest: return GL_NEAREST;
		case MagFilter::Linear:  return GL_LINEAR;
		default: std::unreachable();
		}
	}

	inline GLenum EnumToGL(MinFilter filter) noexcept
	{
		switch (filter)
		{
		case MinFilter::Nearest:             return GL_NEAREST;
		case MinFilter::Linear:              return GL_LINEAR;
		case MinFilter::NearestMimapNearest: return GL_NEAREST_MIPMAP_NEAREST;
		case MinFilter::NearestMimapLinear:  return GL_NEAREST_MIPMAP_LINEAR;
		case MinFilter::LinearMimapNearest:  return GL_LINEAR_MIPMAP_NEAREST;
		case MinFilter::LinearMimapLinear:   return GL_LINEAR_MIPMAP_LINEAR;
		default: std::unreachable();
		}
	}

	inline GLint EnumToGL(AddressMode addressMode) noexcept
	{
		switch (addressMode)
		{
		case AddressMode::Repeat:            return GL_REPEAT;
		case AddressMode::MirroredRepeat:    return GL_MIRRORED_REPEAT;
		case AddressMode::ClampToEdge:       return GL_CLAMP_TO_EDGE;
		case AddressMode::ClampToBorder:     return GL_CLAMP_TO_BORDER;
		case AddressMode::MirrorClampToEdge: return GL_MIRROR_CLAMP_TO_EDGE;
		default: std::unreachable();
		}
	}

	inline GLint EnumToGL(ComponentSwizzle swizzle) noexcept
	{
		switch (swizzle)
		{
		case ComponentSwizzle::Zero: return GL_ZERO;
		case ComponentSwizzle::One:  return GL_ONE;
		case ComponentSwizzle::R:    return GL_RED;
		case ComponentSwizzle::G:    return GL_GREEN;
		case ComponentSwizzle::B:    return GL_BLUE;
		case ComponentSwizzle::A:    return GL_ALPHA;
		default: std::unreachable();
		}
	}

	inline GLsizei EnumToGL(SampleCount sampleCount) noexcept
	{
		switch (sampleCount)
		{
		case SampleCount::Samples1:  return 1;
		case SampleCount::Samples2:  return 2;
		case SampleCount::Samples4:  return 4;
		case SampleCount::Samples8:  return 8;
		case SampleCount::Samples16: return 16;
		case SampleCount::Samples32: return 32;
		default: std::unreachable();
		}
	}

	inline GLenum EnumToGL(ImageType imageType) noexcept
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
		default: std::unreachable();
		}
	}

	inline int ImageTypeToDimension(ImageType imageType) noexcept
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
		default: std::unreachable();
		}
	}

	inline GLenum EnumToGL(Format format) noexcept
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
		default: std::unreachable();
		}
	}
	inline bool IsBlockCompressedFormat(Format format) noexcept
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

	inline GLenum FormatToTypeGL(Format format) noexcept
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
		default: std::unreachable();
		}
	}
	
	inline GlBaseTypeClass FormatToBaseTypeClass(Format format) noexcept
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
		case Format::R3G3B2_UNORM:
		case Format::R4G4B4_UNORM:
		case Format::R5G5B5_UNORM:
		case Format::R8G8B8_UNORM:
		case Format::R8G8B8_SNORM:
		case Format::R10G10B10_UNORM:
		case Format::R12G12B12_UNORM:
		case Format::R16G16B16_SNORM:
		case Format::R2G2B2A2_UNORM:
		case Format::R4G4B4A4_UNORM:
		case Format::R5G5B5A1_UNORM:
		case Format::R8G8B8A8_UNORM:
		case Format::R8G8B8A8_SNORM:
		case Format::R10G10B10A2_UNORM:
		case Format::R12G12B12A12_UNORM:
		case Format::R16G16B16A16_UNORM:
		case Format::R16G16B16A16_SNORM:
		case Format::R8G8B8_SRGB:
		case Format::R8G8B8A8_SRGB:
		case Format::R16_FLOAT:
		case Format::R16G16_FLOAT:
		case Format::R16G16B16_FLOAT:
		case Format::R16G16B16A16_FLOAT:
		case Format::R32_FLOAT:
		case Format::R32G32_FLOAT:
		case Format::R32G32B32_FLOAT:
		case Format::R32G32B32A32_FLOAT:
		case Format::R11G11B10_FLOAT:
		case Format::R9G9B9_E5:
			return GlBaseTypeClass::Float;
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
			return GlBaseTypeClass::SInt;
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
			return GlBaseTypeClass::UInt;
		default: std::unreachable();
		}
	}

	inline bool IsValidImageFormat(Format format) noexcept
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

	inline GLenum EnumToGL(UploadFormat uploadFormat) noexcept
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
		default: std::unreachable();
		}
	}

	inline UploadFormat FormatToUploadFormat(Format format) noexcept
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
		default: std::unreachable();
		}
	}

	inline GLenum EnumToGL(UploadType uploadType) noexcept
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
		default: std::unreachable();
		}
	}

	inline GLenum EnumToGL(PrimitiveTopology topology) noexcept
	{
		switch (topology)
		{
		case PrimitiveTopology::PointList:     return GL_POINTS;
		case PrimitiveTopology::LineList:      return GL_LINES;
		case PrimitiveTopology::LineStrip:     return GL_LINE_STRIP;
		case PrimitiveTopology::TriangleList:  return GL_TRIANGLES;
		case PrimitiveTopology::TriangleStrip: return GL_TRIANGLE_STRIP;
		case PrimitiveTopology::TriangleFan:   return GL_TRIANGLE_FAN;
		case PrimitiveTopology::PatchList:     return GL_PATCHES;
		default: std::unreachable();
		}
	}

	inline GLenum EnumToGL(IndexType type) noexcept
	{
		switch (type)
		{
		case IndexType::UByte:  return GL_UNSIGNED_BYTE;
		case IndexType::UShort: return GL_UNSIGNED_SHORT;
		case IndexType::UInt:   return GL_UNSIGNED_INT;
		default: std::unreachable();
		}
	}
	inline size_t GetIndexSize(IndexType indexType) noexcept
	{
		switch (indexType)
		{
		case IndexType::UByte:  return 1;
		case IndexType::UShort: return 2;
		case IndexType::UInt:   return 4;
		default: std::unreachable();
		}
	}

	inline GLbitfield BarrierBitsToGL(MemoryBarrierBits bits) noexcept
	{
		GLbitfield ret = 0;
		ret |= bits & MemoryBarrierBit::VertexBufferBit ? GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT : 0;
		ret |= bits & MemoryBarrierBit::IndexBufferBit ? GL_ELEMENT_ARRAY_BARRIER_BIT : 0;
		ret |= bits & MemoryBarrierBit::UniformBufferBit ? GL_UNIFORM_BARRIER_BIT : 0;
		ret |= bits & MemoryBarrierBit::TextureFetchBit ? GL_TEXTURE_FETCH_BARRIER_BIT : 0;
		ret |= bits & MemoryBarrierBit::ImageAccessBit ? GL_SHADER_IMAGE_ACCESS_BARRIER_BIT : 0;
		ret |= bits & MemoryBarrierBit::CommandBufferBit ? GL_COMMAND_BARRIER_BIT : 0;
		ret |= bits & MemoryBarrierBit::TextureUpdateBit ? GL_TEXTURE_UPDATE_BARRIER_BIT : 0;
		ret |= bits & MemoryBarrierBit::BufferUpdateBit ? GL_BUFFER_UPDATE_BARRIER_BIT : 0;
		ret |= bits & MemoryBarrierBit::MappedBufferBit ? GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT : 0;
		ret |= bits & MemoryBarrierBit::FramebufferBit ? GL_FRAMEBUFFER_BARRIER_BIT : 0;
		ret |= bits & MemoryBarrierBit::ShaderStorageBit ? GL_SHADER_STORAGE_BARRIER_BIT : 0;
		ret |= bits & MemoryBarrierBit::QueryCounterBit ? GL_QUERY_BUFFER_BARRIER_BIT : 0;
		return ret;
	}

	inline Format FormatToSrgb(Format format) noexcept
	{
		switch (format)
		{
		case Format::BC1_RGBA_UNORM: return Format::BC1_RGBA_SRGB;
		case Format::BC1_RGB_UNORM:  return Format::BC1_RGB_SRGB;
		case Format::BC2_RGBA_UNORM: return Format::BC3_RGBA_SRGB;
		case Format::BC3_RGBA_UNORM: return Format::BC3_RGBA_SRGB;
		case Format::BC7_RGBA_UNORM: return Format::BC7_RGBA_SRGB;
		case Format::R8G8B8A8_UNORM: return Format::R8G8B8A8_SRGB;
		case Format::R8G8B8_UNORM:   return Format::R8G8B8_SRGB;
		default: return format;
		}
	}


} // namespace gl::detail