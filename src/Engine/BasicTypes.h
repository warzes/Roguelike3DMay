#pragma once

// Used to constrain the types accepted by Buffer
class ByteView final : public std::span<const std::byte>
{
public:
	template<typename T> requires std::is_trivially_copyable_v<T>
	constexpr ByteView(const T& t) : std::span<const std::byte>(std::as_bytes(std::span{ &t, static_cast<size_t>(1) })) {}
	template<typename T> requires std::is_trivially_copyable_v<T>
	constexpr ByteView(std::span<const T> t) : std::span<const std::byte>(std::as_bytes(t)) {}
};

template <class T> requires std::is_object_v<T>
class ReferenceWrapper final
{
public:
	using type = T;

	template <class U>
	constexpr ReferenceWrapper(U&& val) noexcept
	{
		T& ref = static_cast<U&&>(val);
		m_ptr = std::addressof(ref);
	}

	[[nodiscard]] constexpr operator T& () const noexcept { return *m_ptr; }
	[[nodiscard]] constexpr T& get() const noexcept { return *m_ptr; }

private:
	T* m_ptr{};
};

struct Extent2D final
{
	constexpr bool operator==(const Extent2D&) const noexcept = default;
	constexpr Extent2D operator+(const Extent2D& other) const noexcept { return { width + other.width, height + other.height }; }
	constexpr Extent2D operator-(const Extent2D& other) const noexcept { return { width - other.width, height - other.height }; }
	constexpr Extent2D operator*(const Extent2D& other) const noexcept { return { width * other.width, height * other.height }; }
	constexpr Extent2D operator/(const Extent2D& other) const noexcept { return { width / other.width, height / other.height }; }
	constexpr Extent2D operator>>(const Extent2D& other) const noexcept { return { width >> other.width, height >> other.height }; }
	constexpr Extent2D operator<<(const Extent2D& other) const noexcept { return { width << other.width, height << other.height }; }
	constexpr Extent2D operator+(uint32_t val) const noexcept { return *this + Extent2D{ val, val }; }
	constexpr Extent2D operator-(uint32_t val) const noexcept { return *this - Extent2D{ val, val }; }
	constexpr Extent2D operator*(uint32_t val) const noexcept { return *this * Extent2D{ val, val }; }
	constexpr Extent2D operator/(uint32_t val) const noexcept { return *this / Extent2D{ val, val }; }
	constexpr Extent2D operator>>(uint32_t val) const noexcept { return *this >> Extent2D{ val, val }; }
	constexpr Extent2D operator<<(uint32_t val) const noexcept { return *this << Extent2D{ val, val }; }

	uint32_t width{ 0 };
	uint32_t height{ 0 };
};

constexpr inline Extent2D operator+(uint32_t val, Extent2D ext) noexcept { return ext + val; }
constexpr inline Extent2D operator-(uint32_t val, Extent2D ext) noexcept { return ext - val; }
constexpr inline Extent2D operator*(uint32_t val, Extent2D ext) noexcept { return ext * val; }
constexpr inline Extent2D operator/(uint32_t val, Extent2D ext) noexcept { return ext / val; }
constexpr inline Extent2D operator>>(uint32_t val, Extent2D ext) noexcept { return ext >> val; }
constexpr inline Extent2D operator<<(uint32_t val, Extent2D ext) noexcept { return ext << val; }

struct Extent3D final
{
	constexpr explicit operator Extent2D() const noexcept { return { width, height }; }
	constexpr bool operator==(const Extent3D&) const noexcept = default;
	constexpr Extent3D operator+(const Extent3D& other) const noexcept { return { width + other.width, height + other.height, depth + other.depth }; }
	constexpr Extent3D operator-(const Extent3D& other) const noexcept { return { width - other.width, height - other.height, depth - other.depth }; }
	constexpr Extent3D operator*(const Extent3D& other) const noexcept { return { width * other.width, height * other.height, depth * other.depth }; }
	constexpr Extent3D operator/(const Extent3D& other) const noexcept { return { width / other.width, height / other.height, depth / other.depth }; }
	constexpr Extent3D operator>>(const Extent3D& other) const noexcept { return { width >> other.width, height >> other.height, depth >> other.depth }; }
	constexpr Extent3D operator<<(const Extent3D& other) const noexcept { return { width << other.width, height << other.height, depth << other.depth }; }
	constexpr Extent3D operator+(uint32_t val) const noexcept { return *this + Extent3D{ val, val, val }; }
	constexpr Extent3D operator-(uint32_t val) const noexcept { return *this - Extent3D{ val, val, val }; }
	constexpr Extent3D operator*(uint32_t val) const noexcept { return *this * Extent3D{ val, val, val }; }
	constexpr Extent3D operator/(uint32_t val) const noexcept { return *this / Extent3D{ val, val, val }; }
	constexpr Extent3D operator>>(uint32_t val) const noexcept { return *this >> Extent3D{ val, val, val }; }
	constexpr Extent3D operator<<(uint32_t val) const noexcept { return *this << Extent3D{ val, val, val }; }

	uint32_t width{ 0 };
	uint32_t height{ 0 };
	uint32_t depth{ 0 };
};

constexpr inline Extent3D operator+(uint32_t val, Extent3D ext) noexcept { return ext + val; }
constexpr inline Extent3D operator-(uint32_t val, Extent3D ext) noexcept { return ext - val; }
constexpr inline Extent3D operator*(uint32_t val, Extent3D ext) noexcept { return ext * val; }
constexpr inline Extent3D operator/(uint32_t val, Extent3D ext) noexcept { return ext / val; }
constexpr inline Extent3D operator>>(uint32_t val, Extent3D ext) noexcept { return ext >> val; }
constexpr inline Extent3D operator<<(uint32_t val, Extent3D ext) noexcept { return ext << val; }

struct Offset2D final
{
	constexpr bool operator==(const Offset2D&) const noexcept = default;
	constexpr Offset2D operator+(const Offset2D& other) const noexcept { return { x + other.x, y + other.y }; }
	constexpr Offset2D operator-(const Offset2D& other) const noexcept { return { x - other.x, y - other.y }; }
	constexpr Offset2D operator*(const Offset2D& other) const noexcept { return { x * other.x, y * other.y }; }
	constexpr Offset2D operator/(const Offset2D& other) const noexcept { return { x / other.x, y / other.y }; }
	constexpr Offset2D operator>>(const Offset2D& other) const noexcept { return { x >> other.x, y >> other.y }; }
	constexpr Offset2D operator<<(const Offset2D& other) const noexcept { return { x << other.x, y << other.y }; }
	constexpr Offset2D operator+(uint32_t val) const noexcept { return *this + Offset2D{ val, val }; }
	constexpr Offset2D operator-(uint32_t val) const noexcept { return *this - Offset2D{ val, val }; }
	constexpr Offset2D operator*(uint32_t val) const noexcept { return *this * Offset2D{ val, val }; }
	constexpr Offset2D operator/(uint32_t val) const noexcept { return *this / Offset2D{ val, val }; }
	constexpr Offset2D operator>>(uint32_t val) const noexcept { return *this >> Offset2D{ val, val }; }
	constexpr Offset2D operator<<(uint32_t val) const noexcept { return *this << Offset2D{ val, val }; }

	uint32_t x{ 0 };
	uint32_t y{ 0 };
};

constexpr inline Offset2D operator+(uint32_t val, Offset2D ext) noexcept { return ext + val; }
constexpr inline Offset2D operator-(uint32_t val, Offset2D ext) noexcept { return ext - val; }
constexpr inline Offset2D operator*(uint32_t val, Offset2D ext) noexcept { return ext * val; }
constexpr inline Offset2D operator/(uint32_t val, Offset2D ext) noexcept { return ext / val; }
constexpr inline Offset2D operator>>(uint32_t val, Offset2D ext) noexcept { return ext >> val; }
constexpr inline Offset2D operator<<(uint32_t val, Offset2D ext) noexcept { return ext << val; }

struct Offset3D final
{
	constexpr explicit operator Offset2D() const noexcept { return { x, y }; }
	constexpr bool operator==(const Offset3D&) const noexcept = default;
	constexpr Offset3D operator+(const Offset3D& other) const noexcept { return { x + other.x, y + other.y, z + other.z }; }
	constexpr Offset3D operator-(const Offset3D& other) const noexcept { return { x - other.x, y - other.y, z - other.z }; }
	constexpr Offset3D operator*(const Offset3D& other) const noexcept { return { x * other.x, y * other.y, z * other.z }; }
	constexpr Offset3D operator/(const Offset3D& other) const noexcept { return { x / other.x, y / other.y, z / other.z }; }
	constexpr Offset3D operator>>(const Offset3D& other) const noexcept { return { x >> other.x, y >> other.y, z >> other.z }; }
	constexpr Offset3D operator<<(const Offset3D& other) const noexcept { return { x << other.x, y << other.y, z << other.z }; }
	constexpr Offset3D operator+(uint32_t val) const noexcept { return *this + Offset3D{ val, val, val }; }
	constexpr Offset3D operator-(uint32_t val) const noexcept { return *this - Offset3D{ val, val, val }; }
	constexpr Offset3D operator*(uint32_t val) const noexcept { return *this * Offset3D{ val, val, val }; }
	constexpr Offset3D operator/(uint32_t val) const noexcept { return *this / Offset3D{ val, val, val }; }
	constexpr Offset3D operator>>(uint32_t val) const noexcept { return *this >> Offset3D{ val, val, val }; }
	constexpr Offset3D operator<<(uint32_t val) const noexcept { return *this << Offset3D{ val, val, val }; }

	uint32_t x{ 0 };
	uint32_t y{ 0 };
	uint32_t z{ 0 };
};

constexpr inline Offset3D operator+(uint32_t val, Offset3D ext) noexcept { return ext + val; }
constexpr inline Offset3D operator-(uint32_t val, Offset3D ext) noexcept { return ext - val; }
constexpr inline Offset3D operator*(uint32_t val, Offset3D ext) noexcept { return ext * val; }
constexpr inline Offset3D operator/(uint32_t val, Offset3D ext) noexcept { return ext / val; }
constexpr inline Offset3D operator>>(uint32_t val, Offset3D ext) noexcept { return ext >> val; }
constexpr inline Offset3D operator<<(uint32_t val, Offset3D ext) noexcept { return ext << val; }

struct Rect2D final
{
	constexpr bool operator==(const Rect2D&) const noexcept = default;

	constexpr bool Contains(Offset2D point) const noexcept
	{
		return point.x >= offset.x && point.y >= offset.y
			&& point.x < offset.x + extent.width
			&& point.y < offset.y + extent.height;
	}

	constexpr bool Intersects(const Rect2D& other) const noexcept
	{
		return offset.x < other.offset.x + other.extent.width
			&& other.offset.x < offset.x + extent.width
			&& offset.y < other.offset.y + other.extent.height
			&& other.offset.y < offset.y + extent.height;
	}

	Offset2D offset{};
	Extent2D extent{};
};