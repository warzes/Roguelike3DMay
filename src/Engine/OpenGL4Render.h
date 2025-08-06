#pragma once

#include "BasicTypes.h"
#include "OpenGL4Core.h"
#include "BasicConstants.h"

namespace gl
{
	class Texture;
	class Sampler;
	class Buffer;
	struct GraphicsPipeline;
	struct ComputePipeline;

	// Minimal reference wrapper type. Didn't want to pull in <functional> just for this
	template <class T>
		requires std::is_object_v<T>
	class ReferenceWrapper final
	{
	public:
		using type = T;

		template <class U>
		constexpr ReferenceWrapper(U&& val) noexcept
		{
			T& ref = static_cast<U&&>(val);
			ptr = std::addressof(ref);
		}

		constexpr operator T& () const noexcept
		{
			return *ptr;
		}

		[[nodiscard]] constexpr T& get() const noexcept
		{
			return *ptr;
		}

	private:
		T* ptr{};
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

	struct ClearDepthStencilValue final
	{
		float depth{};
		int32_t stencil{};
	};

	struct RenderColorAttachment final
	{
		ReferenceWrapper<const Texture> texture;
		AttachmentLoadOp loadOp = AttachmentLoadOp::Load;
		glm::vec4 clearValue;
	};

	struct RenderDepthStencilAttachment final
	{
		ReferenceWrapper<const Texture> texture;
		AttachmentLoadOp loadOp = AttachmentLoadOp::Load;
		ClearDepthStencilValue clearValue;
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

	struct SwapchainRenderInfo final
	{
		/// @brief An optional name to demarcate the pass in a graphics debugger
		std::string_view name;
		Viewport viewport = {};
		AttachmentLoadOp colorLoadOp = AttachmentLoadOp::Load;
		glm::vec4 clearColorValue;
		AttachmentLoadOp depthLoadOp = AttachmentLoadOp::Load;
		float clearDepthValue = 0.0f;
		AttachmentLoadOp stencilLoadOp = AttachmentLoadOp::Load;
		int32_t clearStencilValue = 0;

		/// @brief If true, the linear->nonlinear sRGB OETF will be applied to pixels when rendering to the swapchain
		/// This facility is provided because OpenGL does not expose the swapchain as an image we can interact with in the usual manner.
		bool enableSrgb = true;
	};

	// Describes the render targets that may be used in a draw
	struct RenderInfo final
	{
		/// @brief An optional name to demarcate the pass in a graphics debugger
		std::string_view name;

		/// @brief An optional viewport
		/// If empty, the viewport size will be the minimum the render targets' size and the offset will be 0.
		std::optional<Viewport> viewport = std::nullopt;
		std::span<const RenderColorAttachment> colorAttachments;
		std::optional<RenderDepthStencilAttachment> depthAttachment = std::nullopt;
		std::optional<RenderDepthStencilAttachment> stencilAttachment = std::nullopt;
	};

	// Describes the framebuffer state when rendering with no attachments (e.g., for algorithms that output to images or buffers).
	// Consult the documentation for glFramebufferParameteri for more info.
	struct RenderNoAttachmentsInfo final
	{
		/// @brief An optional name to demarcate the pass in a graphics debugger
		std::string_view name;
		Viewport viewport{};
		Extent3D framebufferSize{}; // If depth > 0, framebuffer is layered
		SampleCount framebufferSamples{};
	};

	/// @brief Renders to the swapchain
	/// @param renderInfo Rendering parameters
	/// The swapchain can be thought of as "the window". This function is provided because OpenGL nor windowing libraries provide a simple mechanism to access the swapchain as a set of images without interop with an explicit API like Vulkan or D3D12.
	void BeginSwapChainRendering(const SwapchainRenderInfo& renderInfo);
	/// @brief Renders to a set of textures
	/// @param renderInfo Rendering parameters
	void BeginRendering(const RenderInfo& renderInfo);
	/// @brief Renders to a virtual texture
	/// @param renderInfo Rendering parameters
	void BeginRenderingNoAttachments(const RenderNoAttachmentsInfo& info);
	void EndRendering();

	void BeginCompute(std::string_view name);
	void EndCompute();

	/// @brief Blits a texture to another texture. Supports minification and magnification
	void BlitTexture(const Texture& source,
		const Texture& target,
		Offset3D sourceOffset,
		Offset3D targetOffset,
		Extent3D sourceExtent,
		Extent3D targetExtent,
		MagFilter filter,
		AspectMask aspect = AspectMaskBit::COLOR_BUFFER_BIT);

	/// @brief Blits a texture to the swapchain. Supports minification and magnification
	void BlitTextureToSwapchain(const Texture& source,
		Offset3D sourceOffset,
		Offset3D targetOffset,
		Extent3D sourceExtent,
		Extent3D targetExtent,
		MagFilter filter,
		AspectMask aspect = AspectMaskBit::COLOR_BUFFER_BIT);

	struct CopyTextureInfo final
	{
		const Texture& source;
		Texture& target;
		uint32_t sourceLevel = 0;
		uint32_t targetLevel = 0;
		Offset3D sourceOffset = {};
		Offset3D targetOffset = {};
		Extent3D extent = {};
	};

	/// @brief Copies data between textures
	/// No format conversion is applied, as in memcpy.
	void CopyTexture(const CopyTextureInfo& copy);

	/// @brief Defines a barrier ordering memory transactions
	/// @param accessBits The barriers to insert
	/// This call is used to ensure that incoherent writes (SSBO writes and image stores) from a shader are reflected in subsequent accesses.
	void MemoryBarrier(MemoryBarrierBits accessBits); // glMemoryBarrier

	/// @brief Allows subsequent draw commands to read the result of texels written in a previous draw operation
	/// See the ARB_texture_barrier spec for potential uses.
	void TextureBarrier(); // glTextureBarrier

	/// @brief Parameters for CopyBuffer()
	struct CopyBufferInfo final
	{
		const Buffer& source;
		Buffer& target;
		uint64_t sourceOffset = 0;
		uint64_t targetOffset = 0;

		/// @brief The amount of data to copy, in bytes. If size is WHOLE_BUFFER, the size of the source buffer is used.
		uint64_t size = WHOLE_BUFFER;
	};

	/// @brief Copies data between buffers
	void CopyBuffer(const CopyBufferInfo& copy);

	/// @brief Parameters for CopyTextureToBuffer
	struct CopyTextureToBufferInfo final
	{
		const Texture& sourceTexture;
		Buffer& targetBuffer;
		uint32_t level = 0;
		Offset3D sourceOffset = {};
		uint64_t targetOffset = {};
		Extent3D extent = {};
		UploadFormat format = UploadFormat::INFER_FORMAT;
		UploadType type = UploadType::INFER_TYPE;

		/// @brief Specifies, in texels, the size of rows in the buffer (for 2D and 3D images). If zero, it is assumed to be tightly packed according to \p extent
		uint32_t bufferRowLength = 0;

		/// @brief Specifies, in texels, the number of rows in the buffer (for 3D images. If zero, it is assumed to be tightly packed according to \p extent
		uint32_t bufferImageHeight = 0;
	};

	/// @brief Copies texture data into a buffer
	void CopyTextureToBuffer(const CopyTextureToBufferInfo& copy);

	struct CopyBufferToTextureInfo final
	{
		const Buffer& sourceBuffer;
		Texture& targetTexture;
		uint32_t level = 0;
		uint64_t sourceOffset = {};
		Offset3D targetOffset = {};
		Extent3D extent = {};

		/// @brief The arrangement of components of texels in the source buffer. DEPTH_STENCIL is not allowed here
		UploadFormat format = UploadFormat::INFER_FORMAT;

		/// @brief The data type of the texel data
		UploadType type = UploadType::INFER_TYPE;

		/// @brief Specifies, in texels, the size of rows in the buffer (for 2D and 3D images). If zero, it is assumed to be tightly packed according to \p extent
		uint32_t bufferRowLength = 0;

		/// @brief Specifies, in texels, the number of rows in the buffer (for 3D images. If zero, it is assumed to be tightly packed according to \p extent
		uint32_t bufferImageHeight = 0;
	};

	/// @brief Copies buffer data into a texture
	void CopyBufferToTexture(const CopyBufferToTextureInfo& copy);

	/*
	id = GraphicsPipeline->Handle() or ComputePipeline->Handle()
	*/
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