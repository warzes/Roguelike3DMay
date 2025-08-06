#pragma once

#include "FlagsUtils.h"
#include "BasicConstants.h"

namespace gl
{
	enum class BufferStorageFlag : uint32_t
	{
		None = 0,
		// Allows the user to update the buffer's contents with UpdateData
		DynamicStorage = 1 << 0,
		// Hints to the implementation to place the buffer storage in host memory
		ClientStorage = 1 << 1,
		// Maps the buffer (persistently and coherently) upon creation
		MapMemory = 1 << 2,
	};
	SE_DECLARE_FLAG_TYPE(BufferStorageFlags, BufferStorageFlag, uint32_t)

	// Used to constrain the types accepted by Buffer
	class TriviallyCopyableByteSpan final : public std::span<const std::byte>
	{
	public:
		template<typename T> requires std::is_trivially_copyable_v<T>
		TriviallyCopyableByteSpan(const T& t)
			: std::span<const std::byte>(std::as_bytes(std::span{ &t, static_cast<size_t>(1) }))
		{
		}

		template<typename T> requires std::is_trivially_copyable_v<T>
		TriviallyCopyableByteSpan(std::span<const T> t) : std::span<const std::byte>(std::as_bytes(t))
		{
		}

		template<typename T> requires std::is_trivially_copyable_v<T>
		TriviallyCopyableByteSpan(std::span<T> t) : std::span<const std::byte>(std::as_bytes(t))
		{
		}
	};

	// Parameters for Buffer::FillData
	struct BufferFillInfo final
	{
		uint64_t offset{ 0 };
		uint64_t size{ WHOLE_BUFFER };
		uint32_t data{ 0 };
	};

	// Encapsulates an OpenGL buffer
	class Buffer
	{
	public:
		explicit Buffer(size_t size, BufferStorageFlags storageFlags = BufferStorageFlag::None, std::string_view name = "");
		explicit Buffer(TriviallyCopyableByteSpan data, BufferStorageFlags storageFlags = BufferStorageFlag::None, std::string_view name = "");

		Buffer(Buffer&& old) noexcept;
		Buffer& operator=(Buffer&& old) noexcept;
		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;
		~Buffer();

		[[nodiscard]] bool IsValid() const noexcept { return m_id > 0; }

		[[nodiscard]] GLuint Handle() const noexcept { return m_id; }
		[[nodiscard]] operator GLuint() const noexcept { return m_id; }

		[[nodiscard]] auto Size() const noexcept { return m_size; }
		[[nodiscard]] bool IsMapped() const noexcept { return m_mappedMemory != nullptr; }

		// Gets a pointer that is mapped to the buffer's data store
		// @return A pointer to mapped memory if the buffer was created with BufferStorageFlag::MapMemory, otherwise nullptr
		[[nodiscard]] void* GetMappedPointer() noexcept { return m_mappedMemory; }
		[[nodiscard]] const void* GetMappedPointer() const noexcept { return m_mappedMemory; }

		void UpdateData(TriviallyCopyableByteSpan data, size_t destOffsetBytes = 0);
		void FillData(const BufferFillInfo& clear = {});	

		// Invalidates the content of the buffer's data store
		// This call can be used to optimize driver synchronization in certain cases.
		void Invalidate();

	protected:
		Buffer(const void* data, size_t size, BufferStorageFlags storageFlags, std::string_view name);

		void updateData(const void* data, size_t size, size_t offset = 0);

		size_t             m_size{};
		BufferStorageFlags m_storageFlags{};
		GLuint             m_id{};
		void*              m_mappedMemory{};
	};

	// A buffer that provides type-safe operations
	// @param T A trivially copyable type
	template<class T>
		requires(std::is_trivially_copyable_v<T>)
	class TypedBuffer final : public Buffer
	{
	public:
		explicit TypedBuffer(BufferStorageFlags storageFlags = BufferStorageFlag::None, std::string_view name = "")
			: Buffer(sizeof(T), storageFlags, name)
		{
		}
		explicit TypedBuffer(size_t count, BufferStorageFlags storageFlags = BufferStorageFlag::None, std::string_view name = "")
			: Buffer(sizeof(T)* count, storageFlags, name)
		{
		}
		explicit TypedBuffer(std::span<const T> data, BufferStorageFlags storageFlags = BufferStorageFlag::None, std::string_view name = "")
			: Buffer(data, storageFlags, name)
		{
		}
		explicit TypedBuffer(const T& data, BufferStorageFlags storageFlags = BufferStorageFlag::None, std::string_view name = "")
			: Buffer(&data, sizeof(T), storageFlags, name)
		{
		}

		TypedBuffer(TypedBuffer&& other) noexcept = default;
		TypedBuffer& operator=(TypedBuffer&& other) noexcept = default;
		TypedBuffer(const TypedBuffer& other) = delete;
		TypedBuffer& operator=(const TypedBuffer&) = delete;

		void UpdateData(const T& data, size_t startIndex = 0)
		{
			Buffer::UpdateData(data, sizeof(T) * startIndex);
		}

		void UpdateData(std::span<const T> data, size_t startIndex = 0)
		{
			Buffer::UpdateData(data, sizeof(T) * startIndex);
		}

		void UpdateData(TriviallyCopyableByteSpan data, size_t destOffsetBytes = 0) = delete;

		[[nodiscard]] T* GetMappedPointer() noexcept { return static_cast<T*>(m_mappedMemory); }
		[[nodiscard]] const T* GetMappedPointer() const noexcept { return static_cast<T*>(m_mappedMemory); }
	};

} // namespace gl