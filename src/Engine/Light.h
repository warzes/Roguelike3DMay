#pragma once

enum class LightType : uint8_t
{
	Point,
	Spot,
	Directional
};

struct LightProperties final
{
	glm::vec3 direction{ 0.f, 0.f, -1.f };
	glm::vec3 color{ 1.f, 1.f, 1.f };
	float intensity{ 1.f };
	float range{ 0.f };
	float inner_cone_angle{ 0.f };
	float outer_cone_angle{ 0.f };
};

class Light final
{
public:
	LightType GetType() const;
	void SetType(LightType type);

	const LightProperties& GetProperties() const;
	void SetProperties(LightProperties& properties);

private:
	LightType m_type;
	LightProperties m_properties;
};