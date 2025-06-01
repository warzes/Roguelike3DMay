#pragma once

namespace math
{
	constexpr const float PI = glm::pi<float>();
	constexpr const float TWOPI = PI * 2;

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