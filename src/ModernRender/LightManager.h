#pragma once

#include "GameLight.h"

class LightManager final
{
public:
	void Free();

	unsigned int CreateNewLight(LightType type, float i);
	GameLight* GetLight(int index);
	void DeleteLight(int index);
	const glm::vec4& GetZenithColor() const;
	void SetZenithColor(const glm::vec4& c);
	unsigned int* GetShadowMapLevelSetting();

private:
	std::unordered_map<unsigned int, GameLight*> m_lights;
	glm::vec4 m_zenithColor{ 0.0f };
	unsigned m_lightID{ 0 };
	unsigned m_shadowMapLevelSetting[3] = { 10, 100, 500 };
};