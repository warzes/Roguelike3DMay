#pragma once

class AABB;

class Frustum final
{
public:
	enum PlaneSide
	{
		Left = 0,
		Right = 1,
		Top = 2,
		Bottom = 3,
		Back = 4,
		Front = 5
	};

	Frustum() = default;
	Frustum(const glm::mat4& matrix);
	~Frustum() = default;

	void Update(const glm::mat4& matrix);

	bool CheckSphere(glm::vec3 pos, float radius);
	bool CheckAABB(const AABB& aabb);

	std::array<glm::vec4, 6> planes;
};