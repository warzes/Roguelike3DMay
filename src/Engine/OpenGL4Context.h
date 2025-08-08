#pragma once

#include "OpenGL4Core.h"
#include "OpenGL4DeviceProperties.h"
#include "BasicConstants.h"
#include "OpenGL4Pipeline.h"
#include "OpenGL4FramebufferCache.h"
#include "OpenGL4VertexArrayCache.h"
#include "OpenGL4SamplerCache.h"
#include "OpenGL4Render.h"

namespace gl
{
	class ContextState final
	{
	public:
		void Init();
		void Close();

		void ResetState();

		// Used for scope error checking
		bool isComputeActive = false;
		bool isRendering = false;

		// Used for error checking for indexed draws
		bool isIndexBufferBound = false;

		// Currently unused
		bool isRenderingToSwapChain = false;

		// True during a render or compute scope that has a name.
		bool isScopedDebugGroupPushed = false;

		// True when a pipeline with a name is bound during a render or compute scope.
		bool isPipelineDebugGroupPushed = false;

		// True during SwapchainRendering scopes that disable sRGB.
		// This is needed since regular Rendering scopes always have framebuffer sRGB enabled (the user uses framebuffer attachments to decide if they want the linear->sRGB conversion).
		bool srgbWasDisabled = false;

		// Stores a pointer to the previously bound graphics pipeline state. This is used for state deduplication.
		// A shared_ptr is needed as the user can delete pipelines at any time, but we need to ensure it stays alive until the next pipeline is bound.
		std::shared_ptr<const detail::GraphicsPipelineInfoOwning> lastGraphicsPipeline{};
		bool lastPipelineWasCompute = false;

		std::shared_ptr<const detail::ComputePipelineInfoOwning> lastComputePipeline{};

		// Currently unused (and probably shouldn't be used)
		const RenderInfo* lastRenderInfo{};

		// These can be set at the start of rendering, so they need to be tracked separately from the other pipeline state.
		std::array<ColorComponentFlags, MAX_COLOR_ATTACHMENTS> lastColorMask = {};
		bool lastDepthMask = true;
		uint32_t lastStencilMask[2] = { static_cast<uint32_t>(-1), static_cast<uint32_t>(-1) };
		bool initViewport = true;
		Viewport lastViewport = {};
		Rect2D lastScissor = {};
		bool scissorEnabled = false;

		// Potentially used for state deduplication.
		GLuint currentVao = 0;
		GLuint currentFbo = 0;

		// These persist until another Pipeline is bound.
		// They are not used for state deduplication, as they are arguments for GL draw calls.
		PrimitiveTopology currentTopology{};
		IndexType currentIndexType{};

		detail::FramebufferCache fboCache;
		detail::VertexArrayCache vaoCache;
		detail::SamplerCache samplerCache;
	} inline gContext;

	/// @brief Invalidates assumptions Engine has made about the OpenGL context state
	/// Call when OpenGL context state has been changed outside of Engine (e.g., when using raw OpenGL or using an external library that calls OpenGL). This invalidates assumptions Engine has made about the pipeline state for the purpose of state deduplication.
	void InvalidatePipelineState();

	// Clears all resource bindings.
	// This is called at the beginning of rendering/compute scopes or when the pipeline state has been invalidated, but only in debug mode.
	void ZeroResourceBindings();

} // namespace gl