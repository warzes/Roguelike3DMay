#include "stdafx.h"
#include "OpenGL4VertexArrayCache.h"
#include "OpenGL4ApiToEnum.h"
#include "OpenGL4Pipeline.h"
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
inline GLboolean isFormatNormalizedGL(gl::Format format)
{
	switch (format)
	{
	case gl::Format::R8_UNORM:
	case gl::Format::R8_SNORM:
	case gl::Format::R16_UNORM:
	case gl::Format::R16_SNORM:
	case gl::Format::R8G8_UNORM:
	case gl::Format::R8G8_SNORM:
	case gl::Format::R16G16_UNORM:
	case gl::Format::R16G16_SNORM:
	case gl::Format::R8G8B8_UNORM:
	case gl::Format::R8G8B8_SNORM:
	case gl::Format::R16G16B16_SNORM:
	case gl::Format::R8G8B8A8_UNORM:
	case gl::Format::R8G8B8A8_SNORM:
	case gl::Format::R16G16B16A16_UNORM:
	case gl::Format::R16G16B16A16_SNORM:
		return GL_TRUE;
	case gl::Format::R16_FLOAT:
	case gl::Format::R32_FLOAT:
	case gl::Format::R8_SINT:
	case gl::Format::R16_SINT:
	case gl::Format::R32_SINT:
	case gl::Format::R8_UINT:
	case gl::Format::R16_UINT:
	case gl::Format::R32_UINT:
	case gl::Format::R16G16_FLOAT:
	case gl::Format::R32G32_FLOAT:
	case gl::Format::R8G8_SINT:
	case gl::Format::R16G16_SINT:
	case gl::Format::R32G32_SINT:
	case gl::Format::R8G8_UINT:
	case gl::Format::R16G16_UINT:
	case gl::Format::R32G32_UINT:
	case gl::Format::R16G16B16_FLOAT:
	case gl::Format::R32G32B32_FLOAT:
	case gl::Format::R8G8B8_SINT:
	case gl::Format::R16G16B16_SINT:
	case gl::Format::R32G32B32_SINT:
	case gl::Format::R8G8B8_UINT:
	case gl::Format::R16G16B16_UINT:
	case gl::Format::R32G32B32_UINT:
	case gl::Format::R16G16B16A16_FLOAT:
	case gl::Format::R32G32B32A32_FLOAT:
	case gl::Format::R8G8B8A8_SINT:
	case gl::Format::R16G16B16A16_SINT:
	case gl::Format::R32G32B32A32_SINT:
	case gl::Format::R10G10B10A2_UINT:
	case gl::Format::R8G8B8A8_UINT:
	case gl::Format::R16G16B16A16_UINT:
	case gl::Format::R32G32B32A32_UINT:
		return GL_FALSE;
	default: std::unreachable();
	}
}
//=============================================================================
inline gl::GlFormatClass formatToFormatClass(gl::Format format)
{
	switch (format)
	{
	case gl::Format::R8_UNORM:
	case gl::Format::R8_SNORM:
	case gl::Format::R16_UNORM:
	case gl::Format::R16_SNORM:
	case gl::Format::R8G8_UNORM:
	case gl::Format::R8G8_SNORM:
	case gl::Format::R16G16_UNORM:
	case gl::Format::R16G16_SNORM:
	case gl::Format::R8G8B8_UNORM:
	case gl::Format::R8G8B8_SNORM:
	case gl::Format::R16G16B16_SNORM:
	case gl::Format::R8G8B8A8_UNORM:
	case gl::Format::R8G8B8A8_SNORM:
	case gl::Format::R16G16B16A16_UNORM:
	case gl::Format::R16G16B16A16_SNORM:
	case gl::Format::R16_FLOAT:
	case gl::Format::R16G16_FLOAT:
	case gl::Format::R16G16B16_FLOAT:
	case gl::Format::R16G16B16A16_FLOAT:
	case gl::Format::R32_FLOAT:
	case gl::Format::R32G32_FLOAT:
	case gl::Format::R32G32B32_FLOAT:
	case gl::Format::R32G32B32A32_FLOAT:
		return gl::GlFormatClass::Float;
	case gl::Format::R8_SINT:
	case gl::Format::R16_SINT:
	case gl::Format::R32_SINT:
	case gl::Format::R8G8_SINT:
	case gl::Format::R16G16_SINT:
	case gl::Format::R32G32_SINT:
	case gl::Format::R8G8B8_SINT:
	case gl::Format::R16G16B16_SINT:
	case gl::Format::R32G32B32_SINT:
	case gl::Format::R8G8B8A8_SINT:
	case gl::Format::R16G16B16A16_SINT:
	case gl::Format::R32G32B32A32_SINT:
	case gl::Format::R10G10B10A2_UINT:
	case gl::Format::R8_UINT:
	case gl::Format::R16_UINT:
	case gl::Format::R32_UINT:
	case gl::Format::R8G8_UINT:
	case gl::Format::R16G16_UINT:
	case gl::Format::R32G32_UINT:
	case gl::Format::R8G8B8_UINT:
	case gl::Format::R16G16B16_UINT:
	case gl::Format::R32G32B32_UINT:
	case gl::Format::R8G8B8A8_UINT:
	case gl::Format::R16G16B16A16_UINT:
	case gl::Format::R32G32B32A32_UINT:
		return gl::GlFormatClass::Int;
	default: std::unreachable();
	}
}
//=============================================================================
inline GLint formatToSizeGL(gl::Format format)
{
	switch (format)
	{
	case gl::Format::R8_UNORM:
	case gl::Format::R8_SNORM:
	case gl::Format::R16_UNORM:
	case gl::Format::R16_SNORM:
	case gl::Format::R16_FLOAT:
	case gl::Format::R32_FLOAT:
	case gl::Format::R8_SINT:
	case gl::Format::R16_SINT:
	case gl::Format::R32_SINT:
	case gl::Format::R8_UINT:
	case gl::Format::R16_UINT:
	case gl::Format::R32_UINT:
		return 1;
	case gl::Format::R8G8_UNORM:
	case gl::Format::R8G8_SNORM:
	case gl::Format::R16G16_FLOAT:
	case gl::Format::R16G16_UNORM:
	case gl::Format::R16G16_SNORM:
	case gl::Format::R32G32_FLOAT:
	case gl::Format::R8G8_SINT:
	case gl::Format::R16G16_SINT:
	case gl::Format::R32G32_SINT:
	case gl::Format::R8G8_UINT:
	case gl::Format::R16G16_UINT:
	case gl::Format::R32G32_UINT:
		return 2;
	case gl::Format::R8G8B8_UNORM:
	case gl::Format::R8G8B8_SNORM:
	case gl::Format::R16G16B16_SNORM:
	case gl::Format::R16G16B16_FLOAT:
	case gl::Format::R32G32B32_FLOAT:
	case gl::Format::R8G8B8_SINT:
	case gl::Format::R16G16B16_SINT:
	case gl::Format::R32G32B32_SINT:
	case gl::Format::R8G8B8_UINT:
	case gl::Format::R16G16B16_UINT:
	case gl::Format::R32G32B32_UINT:
		return 3;
	case gl::Format::R8G8B8A8_UNORM:
	case gl::Format::R8G8B8A8_SNORM:
	case gl::Format::R16G16B16A16_UNORM:
	case gl::Format::R16G16B16A16_SNORM:
	case gl::Format::R16G16B16A16_FLOAT:
	case gl::Format::R32G32B32A32_FLOAT:
	case gl::Format::R8G8B8A8_SINT:
	case gl::Format::R16G16B16A16_SINT:
	case gl::Format::R32G32B32A32_SINT:
	case gl::Format::R10G10B10A2_UINT:
	case gl::Format::R8G8B8A8_UINT:
	case gl::Format::R16G16B16A16_UINT:
	case gl::Format::R32G32B32A32_UINT:
		return 4;
	default: std::unreachable();
	}
}
//=============================================================================
uint32_t gl::detail::VertexArrayCache::CreateOrGetCachedVertexArray(const VertexInputStateOwning& inputState)
{
	auto inputHash = vertexInputStateHash(inputState);
	if (auto it = m_vertexArrayCache.find(inputHash); it != m_vertexArrayCache.end())
		return it->second;

	uint32_t vao{};
	glCreateVertexArrays(1, &vao);
	if (!vao) return 0;

	for (uint32_t i = 0; i < inputState.vertexBindingDescriptions.size(); i++)
	{
		const auto& desc = inputState.vertexBindingDescriptions[i];
		glEnableVertexArrayAttrib(vao, desc.location);
		glVertexArrayAttribBinding(vao, desc.location, desc.binding);

		auto type = detail::FormatToTypeGL(desc.format);
		auto size = formatToSizeGL(desc.format);
		auto normalized = isFormatNormalizedGL(desc.format);
		auto internalType = formatToFormatClass(desc.format);
		switch (internalType)
		{
		case gl::GlFormatClass::Float: glVertexArrayAttribFormat(vao, desc.location, size, type, normalized, desc.offset); break;
		case gl::GlFormatClass::Int:   glVertexArrayAttribIFormat(vao, desc.location, size, type, desc.offset); break;
		case gl::GlFormatClass::Long:  glVertexArrayAttribLFormat(vao, desc.location, size, type, desc.offset); break;
		default: std::unreachable();
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