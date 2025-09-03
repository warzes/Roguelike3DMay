#include "stdafx.h"
#include "OpenGL4Texture.h"
#include "OpenGL4Sampler.h"
#include "OpenGL4ApiToEnum.h"
#include "OpenGL4Context.h"
#include "OpenGL4DeviceProperties.h"
#include "Log.h"
//=============================================================================
inline uint64_t getBlockCompressedImageSize(gl::Format format, uint32_t width, uint32_t height, uint32_t depth)
{
	assert(gl::detail::IsBlockCompressedFormat(format));

	// BCn formats store 4x4 blocks of pixels, even if the dimensions aren't a multiple of 4
	// We round up to the nearest multiple of 4 for width and height, but not depth, since 3D BCn images are just multiple 2D images stacked
	width = (width + 4 - 1) & -4;
	height = (height + 4 - 1) & -4;

	switch (format)
	{
	// BC1 and BC4 store 4x4 blocks with 64 bits (8 bytes)
	case gl::Format::BC1_RGB_UNORM:
	case gl::Format::BC1_RGBA_UNORM:
	case gl::Format::BC1_RGB_SRGB:
	case gl::Format::BC1_RGBA_SRGB:
	case gl::Format::BC4_R_UNORM:
	case gl::Format::BC4_R_SNORM:
		return width * height * depth / 2;

	// BC3, BC5, BC6, and BC7 store 4x4 blocks with 128 bits (16 bytes)
	case gl::Format::BC2_RGBA_UNORM:
	case gl::Format::BC2_RGBA_SRGB:
	case gl::Format::BC3_RGBA_UNORM:
	case gl::Format::BC3_RGBA_SRGB:
	case gl::Format::BC5_RG_UNORM:
	case gl::Format::BC5_RG_SNORM:
	case gl::Format::BC6H_RGB_UFLOAT:
	case gl::Format::BC6H_RGB_SFLOAT:
	case gl::Format::BC7_RGBA_UNORM:
	case gl::Format::BC7_RGBA_SRGB:
		return width * height * depth;
	default: std::unreachable();
	}
}
//=============================================================================
gl::Texture::Texture(const TextureCreateInfo& ci, std::string_view name) : m_createInfo(ci)
{
	glCreateTextures(detail::EnumToGL(ci.imageType), 1, &m_id);

	switch (ci.imageType)
	{
	case ImageType::Tex1D:
		glTextureStorage1D(m_id,
			static_cast<GLsizei>(ci.mipLevels),
			detail::EnumToGL(ci.format),
			static_cast<GLsizei>(ci.extent.width));
		break;
	case ImageType::Tex2D:
		glTextureStorage2D(m_id,
			static_cast<GLsizei>(ci.mipLevels),
			detail::EnumToGL(ci.format),
			static_cast<GLsizei>(ci.extent.width),
			static_cast<GLsizei>(ci.extent.height));
		break;
	case ImageType::Tex3D:
		glTextureStorage3D(m_id,
			static_cast<GLsizei>(ci.mipLevels),
			detail::EnumToGL(ci.format),
			static_cast<GLsizei>(ci.extent.width),
			static_cast<GLsizei>(ci.extent.height),
			static_cast<GLsizei>(ci.extent.depth));
		break;
	case ImageType::Tex1DArray:
		glTextureStorage2D(m_id,
			static_cast<GLsizei>(ci.mipLevels),
			detail::EnumToGL(ci.format),
			static_cast<GLsizei>(ci.extent.width),
			static_cast<GLsizei>(ci.arrayLayers));
		break;
	case ImageType::Tex2DArray:
		glTextureStorage3D(m_id,
			static_cast<GLsizei>(ci.mipLevels),
			detail::EnumToGL(ci.format),
			static_cast<GLsizei>(ci.extent.width),
			static_cast<GLsizei>(ci.extent.height),
			static_cast<GLsizei>(ci.arrayLayers));
		break;
	case ImageType::TexCubemap:
		glTextureStorage2D(m_id,
			static_cast<GLsizei>(ci.mipLevels),
			detail::EnumToGL(ci.format),
			static_cast<GLsizei>(ci.extent.width),
			static_cast<GLsizei>(ci.extent.height));
		break;
	case ImageType::TexCubemapArray:
		glTextureStorage3D(m_id,
			static_cast<GLsizei>(ci.mipLevels),
			detail::EnumToGL(ci.format),
			static_cast<GLsizei>(ci.extent.width),
			static_cast<GLsizei>(ci.extent.height),
			static_cast<GLsizei>(ci.arrayLayers));
		break;
	case ImageType::Tex2DMultisample:
		glTextureStorage2DMultisample(m_id,
			detail::EnumToGL(ci.sampleCount),
			detail::EnumToGL(ci.format),
			static_cast<GLsizei>(ci.extent.width),
			static_cast<GLsizei>(ci.extent.height),
			GL_TRUE);
		break;
	case ImageType::Tex2DMultisampleArray:
		glTextureStorage3DMultisample(m_id,
			detail::EnumToGL(ci.sampleCount),
			detail::EnumToGL(ci.format),
			static_cast<GLsizei>(ci.extent.width),
			static_cast<GLsizei>(ci.extent.height),
			static_cast<GLsizei>(ci.arrayLayers),
			GL_TRUE);
		break;
	default: std::unreachable(); break;
	}

	if (!name.empty())
		glObjectLabel(GL_TEXTURE, m_id, static_cast<GLsizei>(name.length()), name.data());

	Debug("Created Texture with handle " + std::to_string(m_id));
}
//=============================================================================
gl::Texture::Texture(Texture&& old) noexcept
	: m_id(std::exchange(old.m_id, 0)), m_createInfo(old.m_createInfo), m_bindlessHandle(std::exchange(old.m_bindlessHandle, 0))
{
}
//=============================================================================
gl::Texture& gl::Texture::operator=(Texture&& old) noexcept
{
	if (&old == this)
		return *this;
	this->~Texture();
	return *new (this) Texture(std::move(old));
}
//=============================================================================
gl::Texture::~Texture()
{
	destroy();
}
//=============================================================================
gl::TextureView gl::Texture::CreateSingleMipView(uint32_t level)
{
	TextureViewCreateInfo createInfo{
		.viewType = m_createInfo.imageType,
		.format = m_createInfo.format,
		.minLevel = level,
		.numLevels = 1,
		.minLayer = 0,
		.numLayers = m_createInfo.arrayLayers,
	};
	return TextureView(createInfo, *this);
}
//=============================================================================
gl::TextureView gl::Texture::CreateSingleLayerView(uint32_t layer)
{
	TextureViewCreateInfo createInfo{
		.viewType = m_createInfo.imageType,
		.format = m_createInfo.format,
		.minLevel = 0,
		.numLevels = m_createInfo.mipLevels,
		.minLayer = layer,
		.numLayers = 1,
	};
	return TextureView(createInfo, *this);
}
//=============================================================================
gl::TextureView gl::Texture::CreateFormatView(Format newFormat)
{
	TextureViewCreateInfo createInfo{
		.viewType = m_createInfo.imageType,
		.format = newFormat,
		.minLevel = 0,
		.numLevels = m_createInfo.mipLevels,
		.minLayer = 0,
		.numLayers = m_createInfo.arrayLayers,
	};
	return TextureView(createInfo, *this);
}
//=============================================================================
gl::TextureView gl::Texture::CreateSwizzleView(ComponentMapping components)
{
	TextureViewCreateInfo createInfo{
		.viewType = m_createInfo.imageType,
		.format = m_createInfo.format,
		.components = components,
		.minLevel = 0,
		.numLevels = m_createInfo.mipLevels,
		.minLayer = 0,
		.numLayers = m_createInfo.arrayLayers,
	};
	return TextureView(createInfo, *this);
}
//=============================================================================
uint64_t gl::Texture::GetBindlessHandle(const Sampler& sampler)
{
	assert(CurrentDeviceProperties.features.bindlessTextures && "GL_ARB_bindless_texture is not supported");
	assert(m_bindlessHandle == 0 && "Texture already has bindless handle resident.");
	m_bindlessHandle = glGetTextureSamplerHandleARB(m_id, sampler.Handle());
	assert(m_bindlessHandle != 0 && "Failed to create texture sampler handle.");
	glMakeTextureHandleResidentARB(m_bindlessHandle);
	return m_bindlessHandle;
}
//=============================================================================
void gl::Texture::UpdateImage(const TextureUpdateInfo& info)
{
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	subImageInternal(info);
}
//=============================================================================
void gl::Texture::UpdateCompressedImage(const CompressedTextureUpdateInfo& info)
{
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	subCompressedImageInternal(info);
}
//=============================================================================
void gl::Texture::subImageInternal(const TextureUpdateInfo& info)
{
	assert(!detail::IsBlockCompressedFormat(m_createInfo.format));
	GLenum format{};
	if (info.format == UploadFormat::INFER_FORMAT)
	{
		format = detail::EnumToGL(detail::FormatToUploadFormat(m_createInfo.format));
	}
	else
	{
		format = detail::EnumToGL(info.format);
	}

	GLenum type{};
	if (info.type == UploadType::INFER_TYPE)
	{
		type = detail::FormatToTypeGL(m_createInfo.format);
	}
	else
	{
		type = detail::EnumToGL(info.type);
	}

	glPixelStorei(GL_UNPACK_ROW_LENGTH, static_cast<GLint>(info.rowLength));
	glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, static_cast<GLint>(info.imageHeight));

	switch (detail::ImageTypeToDimension(m_createInfo.imageType))
	{
	case 1:
		glTextureSubImage1D(m_id,
			static_cast<GLint>(info.level),
			static_cast<GLint>(info.offset.x),
			static_cast<GLsizei>(info.extent.width),
			format,
			type,
			info.pixels);
		break;
	case 2:
		glTextureSubImage2D(m_id,
			static_cast<GLint>(info.level),
			static_cast<GLint>(info.offset.x),
			static_cast<GLint>(info.offset.y),
			static_cast<GLsizei>(info.extent.width),
			static_cast<GLsizei>(info.extent.height),
			format,
			type,
			info.pixels);
		break;
	case 3:
		glTextureSubImage3D(m_id,
			static_cast<GLint>(info.level),
			static_cast<GLint>(info.offset.x),
			static_cast<GLint>(info.offset.y),
			static_cast<GLint>(info.offset.z),
			static_cast<GLsizei>(info.extent.width),
			static_cast<GLsizei>(info.extent.height),
			static_cast<GLsizei>(info.extent.depth),
			format,
			type,
			info.pixels);
		break;
	}
}
//=============================================================================
void gl::Texture::subCompressedImageInternal(const CompressedTextureUpdateInfo& info)
{
	assert(detail::IsBlockCompressedFormat(m_createInfo.format));
	const GLenum format = detail::EnumToGL(m_createInfo.format);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);

	switch (detail::ImageTypeToDimension(m_createInfo.imageType))
	{
	case 2:
		glCompressedTextureSubImage2D(
			m_id,
			static_cast<GLint>(info.level),
			static_cast<GLint>(info.offset.x),
			static_cast<GLint>(info.offset.y),
			static_cast<GLsizei>(info.extent.width),
			static_cast<GLsizei>(info.extent.height),
			format,
			static_cast<GLsizei>(getBlockCompressedImageSize(m_createInfo.format, info.extent.width, info.extent.height, 1)),
			info.data);
		break;
	case 3:
		glCompressedTextureSubImage3D(
			m_id,
			static_cast<GLint>(info.level),
			static_cast<GLint>(info.offset.x),
			static_cast<GLint>(info.offset.y),
			static_cast<GLint>(info.offset.z),
			static_cast<GLsizei>(info.extent.width),
			static_cast<GLsizei>(info.extent.height),
			static_cast<GLsizei>(info.extent.depth),
			format,
			static_cast<GLsizei>(getBlockCompressedImageSize(m_createInfo.format, info.extent.width, info.extent.height, info.extent.depth)),
			info.data);
		break;
	default: std::unreachable();
	}
}
//=============================================================================
void gl::Texture::destroy()
{
	if (m_id == 0) return;

	if (m_bindlessHandle != 0)
		glMakeTextureHandleNonResidentARB(m_bindlessHandle);

	Debug("Destroyed Texture with handle " + std::to_string(m_id));
	glDeleteTextures(1, &m_id);
	// Ensure that the texture is no longer referenced in the FBO cache
	gContext.fboCache.RemoveTexture(*this);
}
//=============================================================================
void gl::Texture::ClearImage(const TextureClearInfo& info)
{
	// Infer format
	GLenum format{};
	if (info.format == UploadFormat::INFER_FORMAT)
	{
		format = detail::EnumToGL(detail::FormatToUploadFormat(m_createInfo.format));
	}
	else
	{
		format = detail::EnumToGL(info.format);
	}

	// Infer type
	GLenum type{};
	if (info.type == UploadType::INFER_TYPE)
	{
		type = detail::FormatToTypeGL(m_createInfo.format);
	}
	else
	{
		type = detail::EnumToGL(info.type);
	}

	// Infer extent
	Extent3D extent = info.extent;
	if (extent == Extent3D{})
	{
		extent = m_createInfo.extent >> info.level;
		extent.width = std::max(extent.width, 1u);
		extent.height = std::max(extent.height, 1u);
		extent.depth = std::max(extent.depth, 1u);
	}

	glClearTexSubImage(m_id,
		static_cast<GLint>(info.level),
		static_cast<GLint>(info.offset.x),
		static_cast<GLint>(info.offset.y),
		static_cast<GLint>(info.offset.z),
		static_cast<GLsizei>(extent.width),
		static_cast<GLsizei>(extent.height),
		static_cast<GLsizei>(extent.depth),
		format,
		type,
		info.data);
}
//=============================================================================
void gl::Texture::GenMipmaps()
{
	glGenerateTextureMipmap(m_id);
}
//=============================================================================
gl::Texture gl::CreateTexture2D(Extent2D size, Format format, std::string_view name)
{
	return Texture({
			.imageType   = ImageType::Tex2D,
			.format      = format,
			.extent      = {size.width, size.height, 1},
			.mipLevels   = 1u,
			.arrayLayers = 1u,
			.sampleCount = SampleCount::Samples1,
		}, name);
}
//=============================================================================
gl::Texture gl::CreateTexture2DMip(Extent2D size, Format format, uint32_t mipLevels, std::string_view name)
{
	return Texture({
			.imageType   = ImageType::Tex2D,
			.format      = format,
			.extent      = {size.width, size.height, 1},
			.mipLevels   = mipLevels,
			.arrayLayers = 1u,
			.sampleCount = SampleCount::Samples1,
		}, name);
}
//=============================================================================
gl::TextureView::TextureView(const TextureViewCreateInfo& viewInfo, Texture& texture, std::string_view name)
	: m_viewInfo(viewInfo)
{
	m_createInfo = texture.GetCreateInfo();
	glGenTextures(1, &m_id); // glCreateTextures does not work here
	glTextureView(m_id,
		detail::EnumToGL(viewInfo.viewType),
		texture.Handle(),
		detail::EnumToGL(viewInfo.format),
		viewInfo.minLevel,
		viewInfo.numLevels,
		viewInfo.minLayer,
		viewInfo.numLayers);

	glTextureParameteri(m_id, GL_TEXTURE_SWIZZLE_R, detail::EnumToGL(viewInfo.components.r));
	glTextureParameteri(m_id, GL_TEXTURE_SWIZZLE_G, detail::EnumToGL(viewInfo.components.g));
	glTextureParameteri(m_id, GL_TEXTURE_SWIZZLE_B, detail::EnumToGL(viewInfo.components.b));
	glTextureParameteri(m_id, GL_TEXTURE_SWIZZLE_A, detail::EnumToGL(viewInfo.components.a));

	if (!name.empty())
	{
		glObjectLabel(GL_TEXTURE, m_id, static_cast<GLsizei>(name.length()), name.data());
	}

	Print("Created Texture View with handle " + std::to_string(m_id));
}
//=============================================================================
gl::TextureView::TextureView(const TextureViewCreateInfo& viewInfo, TextureView& textureView, std::string_view name)
	: TextureView(viewInfo, static_cast<Texture&>(textureView), name)
{
	m_createInfo = TextureCreateInfo{
		.imageType = textureView.m_viewInfo.viewType,
		.format = textureView.m_viewInfo.format,
		.extent = textureView.m_createInfo.extent,
		.mipLevels = textureView.m_viewInfo.numLevels,
		.arrayLayers = textureView.m_viewInfo.numLayers,
	};
}
//=============================================================================
gl::TextureView::TextureView(Texture& texture, std::string_view name)
	: TextureView(
		TextureViewCreateInfo{
			.viewType = texture.GetCreateInfo().imageType,
			.format = texture.GetCreateInfo().format,
			.minLevel = 0,
			.numLevels = texture.GetCreateInfo().mipLevels,
			.minLayer = 0,
			.numLayers = texture.GetCreateInfo().arrayLayers,
		},
		texture,
		name)
{
}
//=============================================================================
gl::TextureView::TextureView(TextureView&& old) noexcept : Texture(std::move(old)), m_viewInfo(old.m_viewInfo) {}
//=============================================================================
gl::TextureView& gl::TextureView::operator=(TextureView&& old) noexcept
{
	if (&old == this)
		return *this;
	this->~TextureView();
	return *new (this) TextureView(std::move(old));
}
//=============================================================================
gl::TextureView::~TextureView()
{
	destroy();
}
//=============================================================================