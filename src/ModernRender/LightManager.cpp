#include "stdafx.h"
#include "LightManager.h"
//=============================================================================
void LightManager::Free()
{
	for (auto i = m_lights.begin(); i != m_lights.end(); i++)
		delete i->second;

	m_lights.clear();
}
//=============================================================================
unsigned int LightManager::CreateNewLight(LightType type, float i)
{
	unsigned int newLightID = m_lightID;
	m_lightID++;

	GameLight* newLight = new GameLight(type, i);
	m_lights[newLightID] = newLight;

	return newLightID;
}
//=============================================================================
GameLight* LightManager::GetLight(int index)
{
	return m_lights[index];
}
//=============================================================================
void LightManager::DeleteLight(int index)
{
	delete m_lights[index];
	m_lights.erase(index);
}
//=============================================================================
const glm::vec4& LightManager::GetZenithColor() const
{
	return m_zenithColor;
}
//=============================================================================
void LightManager::SetZenithColor(const glm::vec4& c)
{
	m_zenithColor = c;
}
//=============================================================================
unsigned int* LightManager::GetShadowMapLevelSetting()
{
	return m_shadowMapLevelSetting;
}
//=============================================================================