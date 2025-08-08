#include "stdafx.h"
#include "OpenGL4VertexArrayCache.h"
#include "OpenGL4ApiToEnum.h"
#include "OpenGL4PipelineManager.h"
#include "Hash.h"
#include "Log.h"
//=============================================================================
inline size_t vertexInputStateHash(const gl::detail::VertexInputStateOwning& k)
{
	size_t hashVal{};

	for (const auto& desc : k.vertexBindingDescriptions)
	{
		auto cctup = std::make_tuple(desc.location, desc.binding, desc.format, desc.offset);
		auto chashVal = detail::hashing::hash<decltype(cctup)>{}(cctup);
		detail::hashing::hash_combine(hashVal, chashVal);
	}

	return hashVal;
}
//=============================================================================
uint32_t gl::detail::VertexArrayCache::CreateOrGetCachedVertexArray(const VertexInputStateOwning& inputState)
{
	auto inputHash = vertexInputStateHash(inputState);
	if (auto it = m_vertexArrayCache.find(inputHash); it != m_vertexArrayCache.end())
		return it->second;

	uint32_t vao{};
	glCreateVertexArrays(1, &vao);
	for (uint32_t i = 0; i < inputState.vertexBindingDescriptions.size(); i++)
	{
		const auto& desc = inputState.vertexBindingDescriptions[i];
		glEnableVertexArrayAttrib(vao, desc.location);
		glVertexArrayAttribBinding(vao, desc.location, desc.binding);

		auto type = detail::FormatToTypeGL(desc.format);
		auto size = detail::FormatToSizeGL(desc.format);
		auto normalized = detail::IsFormatNormalizedGL(desc.format);
		auto internalType = detail::FormatToFormatClass(desc.format);
		switch (internalType)
		{
		case gl::GlFormatClass::Float: glVertexArrayAttribFormat(vao, desc.location, size, type, normalized, desc.offset); break;
		case gl::GlFormatClass::Int:   glVertexArrayAttribIFormat(vao, desc.location, size, type, desc.offset); break;
		case gl::GlFormatClass::Long:  glVertexArrayAttribLFormat(vao, desc.location, size, type, desc.offset); break;
		default: assert(0);
		}
	}

	Debug("Created Vertex Array with handle " + std::to_string(vao));

	return m_vertexArrayCache.insert({ inputHash, vao }).first->second;
}
//=============================================================================
void gl::detail::VertexArrayCache::Clear()
{
	for (auto [_, vao] : m_vertexArrayCache)
	{
		Debug("Destroyed Vertex Array with handle " + std::to_string(vao));
		glDeleteVertexArrays(1, &vao);
	}

	m_vertexArrayCache.clear();
}
//=============================================================================