#pragma once

class BoundingBox final
{
public:
	BoundingBox() = default;
	BoundingBox(const glm::vec3& Min, const glm::vec3& Max)
		: min(glm::min(Min, Max))
		, max(glm::max(Min, Max))
	{
	}
	BoundingBox(const glm::vec3* points, size_t numPoints)
	{
		glm::vec3 vmin(std::numeric_limits<float>::max());
		glm::vec3 vmax(std::numeric_limits<float>::lowest());

		for (size_t i = 0; i != numPoints; i++)
		{
			vmin = glm::min(vmin, points[i]);
			vmax = glm::max(vmax, points[i]);
		}
		min = vmin;
		max = vmax;
	}

	void Transform(const glm::mat4& t)
	{
		glm::vec3 corners[] = {
			glm::vec3(min.x, min.y, min.z), glm::vec3(min.x, max.y, min.z), glm::vec3(min.x, min.y, max.z), glm::vec3(min.x, max.y, max.z),
			glm::vec3(max.x, min.y, min.z), glm::vec3(max.x, max.y, min.z), glm::vec3(max.x, min.y, max.z), glm::vec3(max.x, max.y, max.z),
		};
		for (auto& v : corners)
			v = glm::vec3(t * glm::vec4(v, 1.0f));
		*this = BoundingBox(corners, 8);
	}
	BoundingBox GetTransformed(const glm::mat4& t) const
	{
		BoundingBox b = *this;
		b.Transform(t);
		return b;
	}
	void CombinePoint(const glm::vec3& p)
	{
		min = glm::min(min, p);
		max = glm::max(max, p);
	}

	glm::vec3 GetSize() const { return glm::vec3(max[0] - min[0], max[1] - min[1], max[2] - min[2]); }
	glm::vec3 GetCenter() const { return 0.5f * glm::vec3(max[0] + min[0], max[1] + min[1], max[2] + min[2]); }

	glm::vec3 min;
	glm::vec3 max;
};

inline BoundingBox CombineBoxes(const std::vector<BoundingBox>& boxes)
{
	std::vector<glm::vec3> allPoints;
	allPoints.reserve(boxes.size() * 8);

	for (const auto& b : boxes)
	{
		allPoints.emplace_back(b.min.x, b.min.y, b.min.z);
		allPoints.emplace_back(b.min.x, b.min.y, b.max.z);
		allPoints.emplace_back(b.min.x, b.max.y, b.min.z);
		allPoints.emplace_back(b.min.x, b.max.y, b.max.z);

		allPoints.emplace_back(b.max.x, b.min.y, b.min.z);
		allPoints.emplace_back(b.max.x, b.min.y, b.max.z);
		allPoints.emplace_back(b.max.x, b.max.y, b.min.z);
		allPoints.emplace_back(b.max.x, b.max.y, b.max.z);
	}

	return BoundingBox(allPoints.data(), allPoints.size());
}

inline bool IsBoxInFrustum(glm::vec4* frustumPlanes, glm::vec4* frustumCorners, const BoundingBox& box)
{
	for (int i = 0; i < 6; i++)
	{
		int r = 0;
		r += (glm::dot(frustumPlanes[i], glm::vec4(box.min.x, box.min.y, box.min.z, 1.0f)) < 0.0) ? 1 : 0;
		r += (glm::dot(frustumPlanes[i], glm::vec4(box.max.x, box.min.y, box.min.z, 1.0f)) < 0.0) ? 1 : 0;
		r += (glm::dot(frustumPlanes[i], glm::vec4(box.min.x, box.max.y, box.min.z, 1.0f)) < 0.0) ? 1 : 0;
		r += (glm::dot(frustumPlanes[i], glm::vec4(box.max.x, box.max.y, box.min.z, 1.0f)) < 0.0) ? 1 : 0;
		r += (glm::dot(frustumPlanes[i], glm::vec4(box.min.x, box.min.y, box.max.z, 1.0f)) < 0.0) ? 1 : 0;
		r += (glm::dot(frustumPlanes[i], glm::vec4(box.max.x, box.min.y, box.max.z, 1.0f)) < 0.0) ? 1 : 0;
		r += (glm::dot(frustumPlanes[i], glm::vec4(box.min.x, box.max.y, box.max.z, 1.0f)) < 0.0) ? 1 : 0;
		r += (glm::dot(frustumPlanes[i], glm::vec4(box.max.x, box.max.y, box.max.z, 1.0f)) < 0.0) ? 1 : 0;
		if (r == 8)
			return false;
	}

	// check frustum outside/inside box
	int r = 0;
	r = 0;
	for (int i = 0; i < 8; i++)
		r += ((frustumCorners[i].x > box.max.x) ? 1 : 0);
	if (r == 8)
		return false;
	r = 0;
	for (int i = 0; i < 8; i++)
		r += ((frustumCorners[i].x < box.min.x) ? 1 : 0);
	if (r == 8)
		return false;
	r = 0;
	for (int i = 0; i < 8; i++)
		r += ((frustumCorners[i].y > box.max.y) ? 1 : 0);
	if (r == 8)
		return false;
	r = 0;
	for (int i = 0; i < 8; i++)
		r += ((frustumCorners[i].y < box.min.y) ? 1 : 0);
	if (r == 8)
		return false;
	r = 0;
	for (int i = 0; i < 8; i++)
		r += ((frustumCorners[i].z > box.max.z) ? 1 : 0);
	if (r == 8)
		return false;
	r = 0;
	for (int i = 0; i < 8; i++)
		r += ((frustumCorners[i].z < box.min.z) ? 1 : 0);
	if (r == 8)
		return false;

	return true;
}