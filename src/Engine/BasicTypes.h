#pragma once

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

template <class T> requires std::is_object_v<T>
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

	constexpr operator T& () const noexcept { return *ptr; }
	[[nodiscard]] constexpr T& get() const noexcept { return *ptr; }

private:
	T* ptr{};
};

struct Extent2D final
{
	bool operator==(const Extent2D&) const noexcept = default;
	Extent2D operator+(const Extent2D& other) const { return { width + other.width, height + other.height }; }
	Extent2D operator-(const Extent2D& other) const { return { width - other.width, height - other.height }; }
	Extent2D operator*(const Extent2D& other) const { return { width * other.width, height * other.height }; }
	Extent2D operator/(const Extent2D& other) const { return { width / other.width, height / other.height }; }
	Extent2D operator>>(const Extent2D& other) const { return { width >> other.width, height >> other.height }; }
	Extent2D operator<<(const Extent2D& other) const { return { width << other.width, height << other.height }; }
	Extent2D operator+(uint32_t val) const { return *this + Extent2D{ val, val }; }
	Extent2D operator-(uint32_t val) const { return *this - Extent2D{ val, val }; }
	Extent2D operator*(uint32_t val) const { return *this * Extent2D{ val, val }; }
	Extent2D operator/(uint32_t val) const { return *this / Extent2D{ val, val }; }
	Extent2D operator>>(uint32_t val) const { return *this >> Extent2D{ val, val }; }
	Extent2D operator<<(uint32_t val) const { return *this << Extent2D{ val, val }; }

	uint32_t width{ 0 };
	uint32_t height{ 0 };
};

inline Extent2D operator+(uint32_t val, Extent2D ext) { return ext + val; }
inline Extent2D operator-(uint32_t val, Extent2D ext) { return ext - val; }
inline Extent2D operator*(uint32_t val, Extent2D ext) { return ext * val; }
inline Extent2D operator/(uint32_t val, Extent2D ext) { return ext / val; }
inline Extent2D operator>>(uint32_t val, Extent2D ext) { return ext >> val; }
inline Extent2D operator<<(uint32_t val, Extent2D ext) { return ext << val; }

struct Extent3D final
{
	operator Extent2D() const { return { width, height }; }
	bool operator==(const Extent3D&) const noexcept = default;
	Extent3D operator+(const Extent3D& other) const { return { width + other.width, height + other.height, depth + other.depth }; }
	Extent3D operator-(const Extent3D& other) const { return { width - other.width, height - other.height, depth - other.depth }; }
	Extent3D operator*(const Extent3D& other) const { return { width * other.width, height * other.height, depth * other.depth }; }
	Extent3D operator/(const Extent3D& other) const { return { width / other.width, height / other.height, depth / other.depth }; }
	Extent3D operator>>(const Extent3D& other) const { return { width >> other.width, height >> other.height, depth >> other.depth }; }
	Extent3D operator<<(const Extent3D& other) const { return { width << other.width, height << other.height, depth << other.depth }; }
	Extent3D operator+(uint32_t val) const { return *this + Extent3D{ val, val, val }; }
	Extent3D operator-(uint32_t val) const { return *this - Extent3D{ val, val, val }; }
	Extent3D operator*(uint32_t val) const { return *this * Extent3D{ val, val, val }; }
	Extent3D operator/(uint32_t val) const { return *this / Extent3D{ val, val, val }; }
	Extent3D operator>>(uint32_t val) const { return *this >> Extent3D{ val, val, val }; }
	Extent3D operator<<(uint32_t val) const { return *this << Extent3D{ val, val, val }; }

	uint32_t width{ 0 };
	uint32_t height{ 0 };
	uint32_t depth{ 0 };
};

inline Extent3D operator+(uint32_t val, Extent3D ext) { return ext + val; }
inline Extent3D operator-(uint32_t val, Extent3D ext) { return ext - val; }
inline Extent3D operator*(uint32_t val, Extent3D ext) { return ext * val; }
inline Extent3D operator/(uint32_t val, Extent3D ext) { return ext / val; }
inline Extent3D operator>>(uint32_t val, Extent3D ext) { return ext >> val; }
inline Extent3D operator<<(uint32_t val, Extent3D ext) { return ext << val; }

struct Offset2D final
{
	bool operator==(const Offset2D&) const noexcept = default;
	Offset2D operator+(const Offset2D& other) const { return { x + other.x, y + other.y }; }
	Offset2D operator-(const Offset2D& other) const { return { x - other.x, y - other.y }; }
	Offset2D operator*(const Offset2D& other) const { return { x * other.x, y * other.y }; }
	Offset2D operator/(const Offset2D& other) const { return { x / other.x, y / other.y }; }
	Offset2D operator>>(const Offset2D& other) const { return { x >> other.x, y >> other.y }; }
	Offset2D operator<<(const Offset2D& other) const { return { x << other.x, y << other.y }; }
	Offset2D operator+(uint32_t val) const { return *this + Offset2D{ val, val }; }
	Offset2D operator-(uint32_t val) const { return *this - Offset2D{ val, val }; }
	Offset2D operator*(uint32_t val) const { return *this * Offset2D{ val, val }; }
	Offset2D operator/(uint32_t val) const { return *this / Offset2D{ val, val }; }
	Offset2D operator>>(uint32_t val) const { return *this >> Offset2D{ val, val }; }
	Offset2D operator<<(uint32_t val) const { return *this << Offset2D{ val, val }; }

	uint32_t x{ 0 };
	uint32_t y{ 0 };
};

inline Offset2D operator+(uint32_t val, Offset2D ext) { return ext + val; }
inline Offset2D operator-(uint32_t val, Offset2D ext) { return ext - val; }
inline Offset2D operator*(uint32_t val, Offset2D ext) { return ext * val; }
inline Offset2D operator/(uint32_t val, Offset2D ext) { return ext / val; }
inline Offset2D operator>>(uint32_t val, Offset2D ext) { return ext >> val; }
inline Offset2D operator<<(uint32_t val, Offset2D ext) { return ext << val; }

struct Offset3D final
{
	operator Offset2D() const { return { x, y }; }
	bool operator==(const Offset3D&) const noexcept = default;
	Offset3D operator+(const Offset3D& other) const { return { x + other.x, y + other.y, z + other.z }; }
	Offset3D operator-(const Offset3D& other) const { return { x - other.x, y - other.y, z - other.z }; }
	Offset3D operator*(const Offset3D& other) const { return { x * other.x, y * other.y, z * other.z }; }
	Offset3D operator/(const Offset3D& other) const { return { x / other.x, y / other.y, z / other.z }; }
	Offset3D operator>>(const Offset3D& other) const { return { x >> other.x, y >> other.y, z >> other.z }; }
	Offset3D operator<<(const Offset3D& other) const { return { x << other.x, y << other.y, z << other.z }; }
	Offset3D operator+(uint32_t val) const { return *this + Offset3D{ val, val, val }; }
	Offset3D operator-(uint32_t val) const { return *this - Offset3D{ val, val, val }; }
	Offset3D operator*(uint32_t val) const { return *this * Offset3D{ val, val, val }; }
	Offset3D operator/(uint32_t val) const { return *this / Offset3D{ val, val, val }; }
	Offset3D operator>>(uint32_t val) const { return *this >> Offset3D{ val, val, val }; }
	Offset3D operator<<(uint32_t val) const { return *this << Offset3D{ val, val, val }; }

	uint32_t x{ 0 };
	uint32_t y{ 0 };
	uint32_t z{ 0 };
};

inline Offset3D operator+(uint32_t val, Offset3D ext) { return ext + val; }
inline Offset3D operator-(uint32_t val, Offset3D ext) { return ext - val; }
inline Offset3D operator*(uint32_t val, Offset3D ext) { return ext * val; }
inline Offset3D operator/(uint32_t val, Offset3D ext) { return ext / val; }
inline Offset3D operator>>(uint32_t val, Offset3D ext) { return ext >> val; }
inline Offset3D operator<<(uint32_t val, Offset3D ext) { return ext << val; }

struct Rect2D final
{
	bool operator==(const Rect2D&) const noexcept = default;

	Offset2D offset{};
	Extent2D extent{};
};