#pragma once

#include "BasicTypes.h"
#include "OpenGL4Core.h"
#include "BasicConstants.h"

namespace gl
{
	class Texture;
	class Buffer;
	
	// Tells what to do with a render target at the beginning of a pass
	enum class AttachmentLoadOp : uint8_t
	{
		Load,     // The previous contents of the image will be preserved
		Clear,    // The contents of the image will be cleared to a uniform value
		DontCare, // The previous contents of the image need not be preserved (they may be discarded)
	};

	struct ClearDepthStencilValue final
	{
		float   depth{ 0.0f };
		int32_t stencil{ 0 };
	};

	struct RenderColorAttachment final
	{
		ReferenceWrapper<const Texture> texture;
		AttachmentLoadOp                loadOp{ AttachmentLoadOp::Load };
		glm::vec4                       clearValue{ 0.0f };
	};

	struct RenderDepthStencilAttachment final
	{
		ReferenceWrapper<const Texture> texture;
		AttachmentLoadOp                loadOp{ AttachmentLoadOp::Load };
		ClearDepthStencilValue          clearValue{ 0.0f };
	};

	struct Viewport final
	{
		Rect2D         drawRect{};                                     // glViewport
		float          minDepth{ 0.0f };                               // glDepthRangef
		float          maxDepth{ 1.0f };                               // glDepthRangef
		ClipDepthRange depthRange{ ClipDepthRange::NegativeOneToOne }; // glClipControl
	};

	struct SwapChainRenderInfo final
	{
		std::string_view name{};
		Viewport         viewport{};
		AttachmentLoadOp colorLoadOp{ AttachmentLoadOp::Load };
		glm::vec4        clearColorValue{ 0.0f };
		AttachmentLoadOp depthLoadOp{ AttachmentLoadOp::Load };
		float            clearDepthValue{ 0.0f };
		AttachmentLoadOp stencilLoadOp{ AttachmentLoadOp::Load };
		int32_t          clearStencilValue{ 0 };
		// If true, the linear->nonlinear sRGB OETF will be applied to pixels when rendering to the swapChain
		// This facility is provided because OpenGL does not expose the swapChain as an image we can interact with in the usual manner.
		bool             enableSrgb{ true };
	};

	// Describes the render targets that may be used in a draw
	struct RenderInfo final
	{
		std::string_view name{};

		// An optional viewport
		// If empty, the viewport size will be the minimum the render targets' size and the offset will be 0.
		std::optional<Viewport>                     viewport{ std::nullopt };
		std::span<const RenderColorAttachment>      colorAttachments;
		std::optional<RenderDepthStencilAttachment> depthAttachment{ std::nullopt };
		std::optional<RenderDepthStencilAttachment> stencilAttachment{ std::nullopt };
	};

	// Describes the framebuffer state when rendering with no attachments (e.g., for algorithms that output to images or buffers).
	// Consult the documentation for glFramebufferParameteri for more info.
	struct RenderNoAttachmentsInfo final
	{
		std::string_view name;
		Viewport         viewport{};
		Extent3D         framebufferSize{}; // If depth > 0, framebuffer is layered
		SampleCount      framebufferSamples{};
	};

	void BeginSwapChainRendering(const SwapChainRenderInfo& renderInfo);
	void BeginRendering(const RenderInfo& renderInfo);
	void BeginRenderingNoAttachments(const RenderNoAttachmentsInfo& info);
	void EndRendering();

	void BeginCompute(std::string_view name);
	void EndCompute();

	// Blits a texture to another texture. Supports minification and magnification
	void BlitTexture(
		const Texture& source,
		const Texture& target,
		Offset3D sourceOffset,
		Offset3D targetOffset,
		Extent3D sourceExtent,
		Extent3D targetExtent,
		MagFilter filter,
		AspectMask aspect = AspectMaskBit::ColorBufferBit);

	// Blits a texture to the swapChain. Supports minification and magnification
	void BlitTextureToSwapChain(
		const Texture& source,
		Offset3D sourceOffset,
		Offset3D targetOffset,
		Extent3D sourceExtent,
		Extent3D targetExtent,
		MagFilter filter,
		AspectMask aspect = AspectMaskBit::ColorBufferBit);

	struct CopyTextureInfo final
	{
		const Texture& source;
		Texture&       target;
		uint32_t       sourceLevel{ 0 };
		uint32_t       targetLevel{ 0 };
		Offset3D       sourceOffset{};
		Offset3D       targetOffset{};
		Extent3D       extent{};
	};
	// Copies data between textures
	// No format conversion is applied, as in memcpy.
	void CopyTexture(const CopyTextureInfo& copy);

	// Defines a barrier ordering memory transactions accessBits The barriers to insert
	// This call is used to ensure that incoherent writes (SSBO writes and image stores) from a shader are reflected in subsequent accesses.
	void MemoryBarrier(MemoryBarrierBits accessBits); // glMemoryBarrier

	// Allows subsequent draw commands to read the result of texels written in a previous draw operation
	// See the ARB_texture_barrier spec for potential uses.
	void TextureBarrier(); // glTextureBarrier

	// Parameters for CopyBuffer()
	struct CopyBufferInfo final
	{
		const Buffer& source;
		Buffer&       target;
		uint64_t      sourceOffset{ 0 };
		uint64_t      targetOffset{ 0 };
		// The amount of data to copy, in bytes. If size is WHOLE_BUFFER, the size of the source buffer is used.
		uint64_t      size{ WHOLE_BUFFER };
	};
	// Copies data between buffers
	void CopyBuffer(const CopyBufferInfo& copy);

	// Parameters for CopyTextureToBuffer
	struct CopyTextureToBufferInfo final
	{
		const Texture& sourceTexture;
		Buffer&        targetBuffer;
		uint32_t       level{ 0 };
		Offset3D       sourceOffset{};
		uint64_t       targetOffset{};
		Extent3D       extent{};
		UploadFormat   format{ UploadFormat::INFER_FORMAT };
		UploadType     type{ UploadType::INFER_TYPE };
		// Specifies, in texels, the size of rows in the buffer (for 2D and 3D images). If zero, it is assumed to be tightly packed according to \p extent
		uint32_t       bufferRowLength{ 0 };
		// Specifies, in texels, the number of rows in the buffer (for 3D images. If zero, it is assumed to be tightly packed according to \p extent
		uint32_t       bufferImageHeight{ 0 };
	};
	// Copies texture data into a buffer
	void CopyTextureToBuffer(const CopyTextureToBufferInfo& copy);

	struct CopyBufferToTextureInfo final
	{
		const Buffer& sourceBuffer;
		Texture&      targetTexture;
		uint32_t      level{ 0 };
		uint64_t      sourceOffset{};
		Offset3D      targetOffset{};
		Extent3D      extent{};
		// The arrangement of components of texels in the source buffer. DEPTH_STENCIL is not allowed here
		UploadFormat  format{ UploadFormat::INFER_FORMAT };
		// The data type of the texel data
		UploadType     type{ UploadType::INFER_TYPE };
		// Specifies, in texels, the size of rows in the buffer (for 2D and 3D images). If zero, it is assumed to be tightly packed according to \p extent
		uint32_t       bufferRowLength{ 0 };
		// Specifies, in texels, the number of rows in the buffer (for 3D images. If zero, it is assumed to be tightly packed according to \p extent
		uint32_t       bufferImageHeight{ 0 };
	};
	// Copies buffer data into a texture
	void CopyBufferToTexture(const CopyBufferToTextureInfo& copy);


	// id = GraphicsPipeline->Handle() or ComputePipeline->Handle()
	// TODO: найти место в интерфейсе для этого
	// TODO: аналогичные методы с целочисленой меткой вместо string
	void SetUniform(uint64_t id, const std::string& label, bool value);
	void SetUniform(uint64_t id, const std::string& label, int value);
	void SetUniform(uint64_t id, const std::string& label, unsigned value);
	void SetUniform(uint64_t id, const std::string& label, float value);
	void SetUniform(uint64_t id, const std::string& label, std::span<const float> value);
	void SetUniform(uint64_t id, const std::string& label, const float* value, int count);
	void SetUniform(uint64_t id, const std::string& label, std::span<const glm::vec2> value);
	void SetUniform(uint64_t id, const std::string& label, std::span<const glm::vec3> value);
	void SetUniform(uint64_t id, const std::string& label, std::span<const glm::vec4> value);
	void SetUniform(uint64_t id, const std::string& label, std::span<const int> value);
	void SetUniform(uint64_t id, const std::string& label, const glm::vec2& value);
	void SetUniform(uint64_t id, const std::string& label, float x, float y);
	void SetUniform(uint64_t id, const std::string& label, const glm::ivec2& value);
	void SetUniform(uint64_t id, const std::string& label, int x, int y);
	void SetUniform(uint64_t id, const std::string& label, const glm::vec3& value);
	void SetUniform(uint64_t id, const std::string& label, float x, float y, float z);
	void SetUniform(uint64_t id, const std::string& label, const glm::vec4& value);
	void SetUniform(uint64_t id, const std::string& label, float x, float y, float z, float w);
	void SetUniform(uint64_t id, const std::string& label, const glm::mat3& mat);
	void SetUniform(uint64_t id, const std::string& label, const glm::mat4& mat);

} // namespace gl