#pragma once

#include "OpenGL4Core.h"
#include "BasicTypes.h"

namespace gl
{
	struct CopyBufferToTextureInfo;
	class Sampler;
	class TextureView;

	struct TextureCreateInfo final
	{
		ImageType   imageType{};
		Format      format{};
		Extent3D    extent{};
		uint32_t    mipLevels{ 0 };
		uint32_t    arrayLayers{ 0 };
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
		// Must be an image type compatible with the base texture as defined by table 8.21 in the OpenGL spec
		ImageType        viewType{};
		// Must be a format compatible with the base texture as defined by table 8.22 in the OpenGL spec
		Format           format{};
		ComponentMapping components{};
		uint32_t         minLevel{ 0 };
		uint32_t         numLevels{ 0 };
		uint32_t         minLayer{ 0 };
		uint32_t         numLayers{ 0 };
	};

	// Parameters for Texture::UpdateImage
	struct TextureUpdateInfo final
	{
		uint32_t     level{ 0 };
		Offset3D     offset{};
		Extent3D     extent{};
		UploadFormat format{ UploadFormat::INFER_FORMAT };
		UploadType   type{ UploadType::INFER_TYPE };
		const void*  pixels{ nullptr };

		// Specifies, in texels, the size of rows in the array (for 2D and 3D images). If zero, it is assumed to be tightly packed according to size
		uint32_t     rowLength{ 0 };

		// Specifies, in texels, the number of rows in the array (for 3D images. If zero, it is assumed to be tightly packed according to size
		uint32_t     imageHeight{ 0 };
	};

	// Parameters for Texture::UpdateCompressedImage
	struct CompressedTextureUpdateInfo final
	{
		uint32_t    level{ 0 };
		Offset3D    offset{};
		Extent3D    extent{};
		const void* data{ nullptr };
	};

	// Parameters for Texture::ClearImage
	struct TextureClearInfo final
	{
		uint32_t     level{ 0 };
		Offset3D     offset{};
		Extent3D     extent{};
		UploadFormat format{ UploadFormat::INFER_FORMAT };
		UploadType   type{ UploadType::INFER_TYPE };
		// If null, then the subresource will be cleared with zeroes
		const void*  data{ nullptr };
	};

	// Encapsulates an immutable OpenGL texture
	class Texture
	{
	public:
		explicit Texture(const TextureCreateInfo& createInfo, std::string_view name = "");
		Texture(Texture&& old) noexcept;
		Texture& operator=(Texture&& old) noexcept;
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;
		~Texture();

		// Updates a subresource of the image
		// info The subresource and data to upload
		// info.format must not be a compressed image format
		void UpdateImage(const TextureUpdateInfo& info);

		// Updates a subresource of the image
		// info The subresource and data to upload
		// Image must be in a compressed image format
		// info.data must be in a compatible compressed image format
		void UpdateCompressedImage(const CompressedTextureUpdateInfo& info);

		// Clears a subresource of the image to a specified value
		// info The subresource and value to clear it with
		void ClearImage(const TextureClearInfo& info);

		// Automatically generates LoDs of the image. All mip levels beyond 0 are filled with the generated LoDs
		void GenMipmaps();

		// Creates a view of a single mip level of the image
		[[nodiscard]] TextureView CreateSingleMipView(uint32_t level);

		// Creates a view of a single array layer of the image
		[[nodiscard]] TextureView CreateSingleLayerView(uint32_t layer);

		// Reinterpret the data of this texture
		// newFormat The format to reinterpret the data as
		// return A new texture view
		[[nodiscard]] TextureView CreateFormatView(Format newFormat);

		// Creates a view of the texture with a new component mapping
		// components The swizzled components
		// return A new texture view
		[[nodiscard]] TextureView CreateSwizzleView(ComponentMapping components);

		// Generates and makes resident a bindless handle from the image and a sampler. Only available if GL_ARB_bindless_texture is supported
		// sampler The sampler to bind to the texture
		// return A bindless texture handle that can be placed in a buffer and used to construct a combined texture sampler in a shader
		[[nodiscard]] uint64_t GetBindlessHandle(const Sampler& sampler);

		[[nodiscard]] const TextureCreateInfo& GetCreateInfo() const noexcept { return m_createInfo; }
		[[nodiscard]] Format GetFormat() const noexcept { return m_createInfo.format; }
		[[nodiscard]] Extent3D GetExtent() const noexcept { return m_createInfo.extent; }

		[[nodiscard]] bool IsValid() const noexcept { return m_id > 0; }

		[[nodiscard]] GLuint Handle() const noexcept { return m_id; }
		[[nodiscard]] operator GLuint() const noexcept { return m_id; }

	protected:
		friend void CopyBufferToTexture(const CopyBufferToTextureInfo& copy);

		void subImageInternal(const TextureUpdateInfo& info);
		void subCompressedImageInternal(const CompressedTextureUpdateInfo& info);
		void destroy();

		Texture() = default;
		uint32_t          m_id{};
		TextureCreateInfo m_createInfo{};
		uint64_t          m_bindlessHandle{ 0 };
	};

	Texture CreateTexture2D(Extent2D size, Format format, std::string_view name = "");
	Texture CreateTexture2DMip(Extent2D size, Format format, uint32_t mipLevels, std::string_view name = "");

	class TextureView final : public Texture
	{
	public:
		explicit TextureView(const TextureViewCreateInfo& viewInfo, Texture& texture, std::string_view name = "");
		explicit TextureView(const TextureViewCreateInfo& viewInfo, TextureView& textureView, std::string_view name = "");
		explicit TextureView(Texture& texture, std::string_view name = "");

		TextureView(TextureView&& old) noexcept;
		TextureView& operator=(TextureView&& old) noexcept;
		TextureView(const TextureView& other) = delete;
		TextureView& operator=(const TextureView& other) = delete;
		~TextureView();

		[[nodiscard]] const TextureViewCreateInfo& GetViewInfo() const noexcept { return m_viewInfo; }

	private:
		TextureView() = default;
		TextureViewCreateInfo m_viewInfo{};
	};

} // namespace gl