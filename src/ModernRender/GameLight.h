#pragma once

#include "GameObject.h"

enum class LightType : uint8_t
{
	Direction,
	Point,
	Spot
};

class GameLight final : public GameObject
{
public:
	GameLight(LightType type, float intensity);

	LightType GetType() const;
	float GetIntensity() const;
	void SetIntensity(float intensity);
	const glm::vec3& GetColor() const;
	void SetColor(const glm::vec3& color);

	void SetNearClip(float nearDis);
	float GetNearClip() const;
	void SetFarClip(float farDis);
	float GetFarClip() const;
	void SetLightSize(float size);
	float GetLightSize() const;
	void SetShadowMapSize(int size);
	int GetShadowMapSize() const;
	const glm::vec3& GetDepthClampPara() const;

private:
	LightType m_lightType;
	float     m_intensity{ 0.0f };
	glm::vec3 m_color{ 0.0f };

	float     m_nearClip{ 0.0f };
	float     m_farClip{ 0.0f };
	float     m_lightSize{ 0.0f };
	int       m_shadowMapSize{ 0 };
};