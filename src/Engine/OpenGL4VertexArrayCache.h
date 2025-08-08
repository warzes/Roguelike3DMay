#pragma once

namespace gl::detail
{
	struct VertexInputStateOwning;

	class VertexArrayCache final
	{
	public:
		VertexArrayCache() = default;
		VertexArrayCache(const VertexArrayCache&) = delete;
		VertexArrayCache& operator=(const VertexArrayCache&) = delete;
		VertexArrayCache(VertexArrayCache&&) noexcept = default;
		VertexArrayCache& operator=(VertexArrayCache&&) noexcept = default;

		uint32_t CreateOrGetCachedVertexArray(const VertexInputStateOwning& inputState);
		[[nodiscard]] size_t Size() const { return m_vertexArrayCache.size(); }
		void Clear();

	private:
		std::unordered_map<size_t, uint32_t> m_vertexArrayCache;
	};
} // namespace gl::detail