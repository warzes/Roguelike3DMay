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

	inline float Floor(float a)
	{
		return floorf(a);
	}

	inline glm::vec3 Floor(const glm::vec3& a)
	{
		return glm::vec3(floorf(a.x), floorf(a.y), floorf(a.z));
	}

	inline glm::vec4 Floor(const glm::vec4& a)
	{
		return glm::vec4(floorf(a.x), floorf(a.y), floorf(a.z), floorf(a.w));
	}

	inline float Fract(float a)
	{
		return a - Floor(a);
	}

	inline glm::vec3 Fract(const glm::vec3& a)
	{
		return a - Floor(a);
	}

	inline glm::vec4 Fract(const glm::vec4& a)
	{
		return a - Floor(a);
	}

	inline float Step(float a, float b)
	{
		return a >= b ? 1.0f : 0.0f;
	}

	inline glm::vec3 Step(const glm::vec3& a, float b)
	{
		return glm::vec3(a.x >= b ? 1.0f : 0.0f,
			a.y >= b ? 1.0f : 0.0f,
			a.z >= b ? 1.0f : 0.0f);
	}

	inline glm::vec4 Step(const glm::vec4& a, float b)
	{
		return glm::vec4(a.x >= b ? 1.0f : 0.0f,
			a.y >= b ? 1.0f : 0.0f,
			a.z >= b ? 1.0f : 0.0f,
			a.w >= b ? 1.0f : 0.0f);
	}

	inline glm::vec3 Step(const glm::vec3& a, const glm::vec3& b)
	{
		return glm::vec3(a.x >= b.x ? 1.0f : 0.0f,
			a.y >= b.y ? 1.0f : 0.0f,
			a.z >= b.z ? 1.0f : 0.0f);
	}

	inline glm::vec4 Step(const glm::vec4& a, const glm::vec4& b)
	{
		return glm::vec4(a.x >= b.x ? 1.0f : 0.0f,
			a.y >= b.y ? 1.0f : 0.0f,
			a.z >= b.z ? 1.0f : 0.0f,
			a.w >= b.w ? 1.0f : 0.0f);
	}

	inline float Remap(float a, float oldMin, float oldMax, float newMin, float newMax)
	{
		return newMin + (newMax - newMin) * (a - oldMin) / (oldMax - oldMin);
	}

	inline glm::vec4 xyY2RGB(const glm::vec3& xyY)
	{
		//xyY to XYZ
		glm::vec3 XYZ = glm::vec3(xyY.x * xyY.z / xyY.y, xyY.z, (1.0f - xyY.x - xyY.y) * xyY.z / xyY.y);

		//XYZ to rgb
		return glm::vec4(3.240479f * XYZ.x - 1.53715f * XYZ.y - 0.49853f * XYZ.z,
			-0.969256f * XYZ.x + 1.875991f * XYZ.y + 0.041556f * XYZ.z,
			0.055648f * XYZ.x - 0.204043f * XYZ.y + 1.057311f * XYZ.z,
			1.0f);
	}

	constexpr inline bool IsPowerOfTwo(int const value)
	{
		return (value != 0) && ((value & (value - 1)) == 0);
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