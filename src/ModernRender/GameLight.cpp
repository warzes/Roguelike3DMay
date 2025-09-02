#include "stdafx.h"
#include "GameLight.h"
//=============================================================================
GameLight::GameLight(LightType type, float intensity)
{
	m_type = GameObjectType::Light;
	m_lightType = type;
	SetIntensity(intensity);
	m_lightSize = 1.0f;
	m_nearClip = 1.0f;
	m_farClip = 100.0f;
	m_shadowMapSize = 2048;
}
//=============================================================================
LightType GameLight::GetType() const
{
	return m_lightType;
}
//=============================================================================
float GameLight::GetIntensity() const
{
	return m_intensity;
}
//=============================================================================
void GameLight::SetIntensity(float intensity)
{
	m_intensity = glm::max(0.0f, intensity);
}
//=============================================================================
const glm::vec3& GameLight::GetColor() const
{
	return m_color * m_intensity;
}
//=============================================================================
void GameLight::SetColor(const glm::vec3& color)
{
	m_color = color;
}
//=============================================================================
void GameLight::SetNearClip(float nearDis)
{
	m_nearClip = nearDis;
}
//=============================================================================
float GameLight::GetNearClip() const
{
	return m_nearClip;
}
//=============================================================================
void GameLight::SetFarClip(float farDis)
{
	m_farClip = farDis;
}
//=============================================================================
float GameLight::GetFarClip() const
{
	return m_farClip;
}
//=============================================================================
void GameLight::SetLightSize(float size)
{
	m_lightSize = size;
}
//=============================================================================
float GameLight::GetLightSize() const
{
	return m_lightSize;
}
//=============================================================================
void GameLight::SetShadowMapSize(int size)
{
	m_shadowMapSize = size;
}
//=============================================================================
int GameLight::GetShadowMapSize() const
{
	return m_shadowMapSize;
}
//=============================================================================
const glm::vec3& GameLight::GetDepthClampPara() const
{
	return glm::vec3(m_nearClip, m_farClip, 1.0f / m_farClip);
}
//=============================================================================