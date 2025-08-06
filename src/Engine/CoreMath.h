#pragma once

namespace math
{
	constexpr const float PI = glm::pi<float>();
	constexpr const float HALF_PI = PI * 0.5f;
	constexpr const float TWO_PI = PI * 2.0f;
	constexpr const float LN2 = 0.6931471805599453094f;

	template <typename T>
	constexpr T Max()
	{
		return std::numeric_limits<T>::max();
	}

	template <typename T>
	constexpr T Min()
	{
		return std::numeric_limits<T>::min();
	}

	template <typename T>
	constexpr T Nan()
	{
		return std::numeric_limits<T>::quiet_NaN();
	}

	template <typename T>
	bool IsNan(T const& x)
	{
		return std::isnan(x);
	}

	constexpr inline bool IsPowerOfTwo(int const value)
	{
		return (value != 0) && ((value & (value - 1)) == 0);
	}

	inline int UpperPowerOfTwo(int const value)
	{
		return static_cast<int>(std::pow(2.0f, std::ceil(std::log(static_cast<float>(value)) / LN2)));
	}

	inline int LowerPowerOfTwo(int const value)
	{
		return static_cast<int>(std::pow(2.0f, std::floor(std::log(static_cast<float>(value)) / LN2)));
	}

	inline int NearestPowerOfTwo(int const value)
	{
		return static_cast<int>(std::pow(2.0f, std::round(std::log(static_cast<float>(value)) / LN2)));
	}

	inline void GetFrustumPlanes(glm::mat4 viewProj, glm::vec4* planes)
	{
		viewProj = glm::transpose(viewProj);
		planes[0] = glm::vec4(viewProj[3] + viewProj[0]); // left
		planes[1] = glm::vec4(viewProj[3] - viewProj[0]); // right
		planes[2] = glm::vec4(viewProj[3] + viewProj[1]); // bottom
		planes[3] = glm::vec4(viewProj[3] - viewProj[1]); // top
		planes[4] = glm::vec4(viewProj[3] + viewProj[2]); // near
		planes[5] = glm::vec4(viewProj[3] - viewProj[2]); // far
	}

	inline void GetFrustumCorners(glm::mat4 viewProj, glm::vec4* points)
	{
		const glm::vec4 corners[] = { 
			glm::vec4(-1, -1, -1, 1), glm::vec4(1, -1, -1, 1), glm::vec4(1, 1, -1, 1), glm::vec4(-1, 1, -1, 1), 
			glm::vec4(-1, -1, 1, 1),  glm::vec4(1, -1, 1, 1),  glm::vec4(1, 1, 1, 1),  glm::vec4(-1, 1, 1, 1) };

		const glm::mat4 invViewProj = glm::inverse(viewProj);

		for (int i = 0; i != 8; i++)
		{
			const glm::vec4 q = invViewProj * corners[i];
			points[i] = q / q.w;
		}
	}
} // namespace math