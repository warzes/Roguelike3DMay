#pragma once

#include "OpenGL4Render.h"

namespace gl
{
	class Sampler;
	struct GraphicsPipeline;
	struct ComputePipeline;

	// Functions that set pipeline state, binds resources, or issues draws or dispatches
	// These functions are analogous to Vulkan vkCmd* calls, which can only be made inside of an active command buffer.
	// @note Calling functions in this namespace outside of a rendering or compute scope will result in undefined behavior
	namespace Cmd
	{
		void BindGraphicsPipeline(const GraphicsPipeline& pipeline);
		void BindComputePipeline(const ComputePipeline& pipeline);

		void SetViewport(const Viewport& viewport);
		void SetScissor(const Rect2D& scissor);

		// Equivalent to glDrawArraysInstancedBaseInstance or vkCmdDraw
		//	 vertexCount The number of vertices to draw
		//	 instanceCount The number of instances to draw
		//	 firstVertex The index of the first vertex to draw
		//	 firstInstance The instance ID of the first instance to draw
		void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);

		// Equivalent to glDrawElementsInstancedBaseVertexBaseInstance or vkCmdDrawIndexed
		//	 indexCount The number of vertices to draw
		//	 instanceCount The number of instances to draw
		//	 firstIndex The base index within the index buffer
		//	 vertexOffset The value added to the vertex index before indexing into the vertex buffer
		//	 firstInstance The instance ID of the first instance to draw
		void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);

		// Equivalent to glMultiDrawArraysIndirect or vkCmdDrawDrawIndirect
		//	 commandBuffer The buffer containing draw parameters
		//	 commandBufferOffset The byte offset into commandBuffer where parameters begin
		//	 drawCount The number of draws to execute
		//	 stride The byte stride between successive sets of draw parameters
		void DrawIndirect(const Buffer& commandBuffer, uint64_t commandBufferOffset, uint32_t drawCount, uint32_t stride);

		// Equivalent to glMultiDrawArraysIndirectCount or vkCmdDrawIndirectCount
		//	 commandBuffer The buffer containing draw parameters
		//	 commandBufferOffset The byte offset into commandBuffer where parameters begin
		//	 countBuffer The buffer containing the draw count
		//	 countBufferOffset The byte offset into countBuffer where the draw count begins
		//	 maxDrawCount The maximum number of draws that will be executed
		//	 stride The byte stride between successive sets of draw parameters
		void DrawIndirectCount(const Buffer& commandBuffer, uint64_t commandBufferOffset, const Buffer& countBuffer, uint64_t countBufferOffset, uint32_t maxDrawCount, uint32_t stride);

		// Equivalent to glMultiDrawElementsIndirect or vkCmdDrawIndexedIndirect
		//	 commandBuffer The buffer containing draw parameters
		//	 commandBufferOffset The byte offset into commandBuffer where parameters begin
		//	 drawCount The number of draws to execute
		//	 stride The byte stride between successive sets of draw parameters
		void DrawIndexedIndirect(const Buffer& commandBuffer, uint64_t commandBufferOffset, uint32_t drawCount, uint32_t stride);

		// Equivalent to glMultiDrawElementsIndirectCount or vkCmdDrawIndexedIndirectCount
		//	 commandBuffer The buffer containing draw parameters
		//	 commandBufferOffset The byte offset into commandBuffer where parameters begin
		//	 countBuffer The buffer containing the draw count
		//	 countBufferOffset The byte offset into countBuffer where the draw count begins
		//	 maxDrawCount The maximum number of draws that will be executed
		//	 stride The byte stride between successive sets of draw parameters
		void DrawIndexedIndirectCount(const Buffer& commandBuffer, uint64_t commandBufferOffset, const Buffer& countBuffer, uint64_t countBufferOffset, uint32_t maxDrawCount, uint32_t stride);

		// Binds a buffer to a vertex buffer binding point
		// Similar to glVertexArrayVertexBuffer. Valid in rendering scopes.
		void BindVertexBuffer(uint32_t bindingIndex, const Buffer& buffer, uint64_t offset, uint64_t stride);

		// Binds an index buffer
		// Similar to glVertexArrayElementBuffer. Valid in rendering scopes.
		void BindIndexBuffer(const Buffer& buffer, IndexType indexType);

		//f Binds a range within a buffer as a uniform buffer
		// Similar to glBindBufferRange(GL_UNIFORM_BUFFER, ...)
		void BindUniformBuffer(uint32_t index, const Buffer& buffer, uint64_t offset = 0, uint64_t size = WHOLE_BUFFER);

		// Binds a range within a buffer as a uniform buffer
		//	 block The name of the uniform block whose index to bind to
		// note: Must be called after a pipeline is bound in order to get reflected program info
		void BindUniformBuffer(std::string_view block, const Buffer& buffer, uint64_t offset = 0, uint64_t size = WHOLE_BUFFER);

		// Binds a range within a buffer as a storage buffer
		// Similar to glBindBufferRange(GL_SHADER_STORAGE_BUFFER, ...)
		void BindStorageBuffer(uint32_t index, const Buffer& buffer, uint64_t offset = 0, uint64_t size = WHOLE_BUFFER);

		// Binds a range within a buffer as a storage buffer
		//	 block The name of the storage block whose index to bind to
		// note: Must be called after a pipeline is bound in order to get reflected program info
		void BindStorageBuffer(std::string_view block, const Buffer& buffer, uint64_t offset = 0, uint64_t size = WHOLE_BUFFER);

		// Binds a texture and a sampler to a texture unit
		// Similar to glBindTextureUnit + glBindSampler
		void BindSampledImage(uint32_t index, const Texture& texture, const Sampler& sampler);

		// Binds a texture and a sampler to a texture unit
		//	 uniform The name of the uniform whose index to bind to
		// note Must be called after a pipeline is bound in order to get reflected program info
		void BindSampledImage(std::string_view uniform, const Texture& texture, const Sampler& sampler);

		// Binds a texture to an image unit
		// Similar to glBindImageTexture{s}
		void BindImage(uint32_t index, const Texture& texture, uint32_t level);

		// Binds a texture to an image unit
		//	 uniform The name of the uniform whose index to bind to
		// note: Must be called after a pipeline is bound in order to get reflected program info
		void BindImage(std::string_view uniform, const Texture& texture, uint32_t level);

		// Invokes a compute shader
		void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
		void Dispatch(Extent3D groupCount);

		// Invokes a compute shader a specified number of times
		// param invocationCountX The minimum number of invocations in the X dimension
		// param invocationCountY The minimum number of invocations in the Y dimension
		// param invocationCountZ The minimum number of invocations in the Z dimension
		// Automatically computes the number of workgroups to invoke based on the formula 
		// groupCount = (invocationCount + workgroupSize - 1) / workgroupSize.
		void DispatchInvocations(uint32_t invocationCountX, uint32_t invocationCountY, uint32_t invocationCountZ);
		void DispatchInvocations(Extent3D invocationCount);

		// Invokes a compute shader with at least as many threads as there are pixels in the image
		// param texture The texture from which to infer the dispatch size
		// param lod The level of detail of the tetxure from which to infer the dispatch size
		// Automatically computes the number of workgroups to invoke based on the formula
		// groupCount = (invocationCount + workgroupSize - 1) / workgroupSize.
		// For 3D images, the depth is used for the Z component of invocationCount. Otherwise,
		// the number of array layers will be used.
		// For cube textures, the Z component of invocationCount will be equal to 6 times
		// the number of array layers.
		void DispatchInvocations(const Texture& texture, uint32_t lod = 0);

		// Invokes a compute shader with the group count provided by a buffer
		// param commandBuffer The buffer containing dispatch parameters
		// param commandBufferOffset The byte offset into commandBuffer where the parameters begin
		void DispatchIndirect(const Buffer& commandBuffer, uint64_t commandBufferOffset);

	} // namespace Cmd

	namespace Cmd
	{
		// TODO: доделать
	/*	inline void BindSampledImage(uint32_t index, std::optional<Texture> texture, std::optional<Sampler> sampler)
		{
			assert(texture.has_value());
			assert(sampler.has_value());
			BindSampledImage(index, texture.value(), sampler.value());
		}*/
	} // namespace Cmd

} // namespace gl