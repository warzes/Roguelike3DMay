#include "stdafx.h"
#include "OpenGL4Buffer.h"
#include "OpenGL4ApiToEnum.h"
#include "Log.h"
//=============================================================================
namespace
{
	constexpr size_t roundUp(size_t numberToRoundUp, size_t multipleOf)
	{
		assert(multipleOf);
		return ((numberToRoundUp + multipleOf - 1) / multipleOf) * multipleOf;
	}
}
//=============================================================================
gl::Buffer::Buffer(const void* data, size_t size, BufferStorageFlags storageFlags, std::string_view name)
	: m_size(roundUp(size, 16))
	, m_storageFlags(storageFlags)
{
	GLbitfield glflags = detail::BufferStorageFlagsToGL(storageFlags);
	glCreateBuffers(1, &m_id);
	glNamedBufferStorage(m_id, static_cast<GLsizeiptr>(m_size), data, glflags);
	if (storageFlags & BufferStorageFlag::MapMemory)
	{
		// GL_MAP_UNSYNCHRONIZED_BIT should be used if the user can map and unmap buffers at their own will
		constexpr GLenum access = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		m_mappedMemory = glMapNamedBufferRange(m_id, 0, static_cast<GLsizeiptr>(m_size), access);
	}

	if (!name.empty())
	{
		glObjectLabel(GL_BUFFER, m_id, static_cast<GLsizei>(name.length()), name.data());
	}

	Debug("Created Buffer with handle " + std::to_string(m_id));
}
//=============================================================================
gl::Buffer::Buffer(size_t size, BufferStorageFlags storageFlags, std::string_view name)
	: Buffer(nullptr, size, storageFlags, name)
{
}
//=============================================================================
gl::Buffer::Buffer(TriviallyCopyableByteSpan data, BufferStorageFlags storageFlags, std::string_view name)
	: Buffer(data.data(), data.size_bytes(), storageFlags, name)
{
}
//=============================================================================
gl::Buffer::Buffer(Buffer&& old) noexcept
	: m_size(std::exchange(old.m_size, 0)),
	m_storageFlags(std::exchange(old.m_storageFlags, BufferStorageFlag::None)),
	m_id(std::exchange(old.m_id, 0)),
	m_mappedMemory(std::exchange(old.m_mappedMemory, nullptr))
{
}
//=============================================================================
gl::Buffer& gl::Buffer::operator=(Buffer&& old) noexcept
{
	if (&old == this)
		return *this;
	this->~Buffer();
	return *new (this) Buffer(std::move(old));
}
//=============================================================================
gl::Buffer::~Buffer()
{
	if (m_id)
	{
		Debug("Destroyed Buffer with handle " + std::to_string(m_id));

		if (m_mappedMemory)
		{
			glUnmapNamedBuffer(m_id);
		}
		glDeleteBuffers(1, &m_id);
	}
}
//=============================================================================
void gl::Buffer::UpdateData(TriviallyCopyableByteSpan data, size_t destOffsetBytes)
{
	updateData(data.data(), data.size_bytes(), destOffsetBytes);
}
//=============================================================================
void gl::Buffer::updateData(const void* data, size_t size, size_t offset)
{
	assert((m_storageFlags & BufferStorageFlag::DynamicStorage) && "UpdateData can only be called on buffers created with the DynamicStorage flag");
	assert(size + offset <= Size());
	glNamedBufferSubData(m_id, static_cast<GLuint>(offset), static_cast<GLuint>(size), data);
}
//=============================================================================
void gl::Buffer::FillData(const BufferFillInfo& clear)
{
	const auto actualSize = clear.size == WHOLE_BUFFER ? m_size : clear.size;
	assert(actualSize % 4 == 0 && "Size must be a multiple of 4 bytes");
	glClearNamedBufferSubData(m_id,
		GL_R32UI,
		static_cast<GLintptr>(clear.offset),
		static_cast<GLsizeiptr>(actualSize),
		GL_RED_INTEGER,
		GL_UNSIGNED_INT,
		&clear.data);
}
//=============================================================================
void gl::Buffer::Invalidate()
{
	glInvalidateBufferData(m_id);
}
//=============================================================================