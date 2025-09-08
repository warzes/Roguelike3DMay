#include "stdafx.h"
#include "DirectionalLight.h"
//=============================================================================
void DirectionalLight::SetPosition(const glm::vec3& position, const glm::vec3& targetView)
{
	m_position = position;
	m_targetView = targetView;
	m_needsUpdate = true;
}
//=============================================================================
void DirectionalLight::SetColor(const glm::vec3& color, float intensity)
{
	m_color = color;
	m_intensity = intensity;
}
//=============================================================================
const glm::vec3& DirectionalLight::GetPosition() const
{
	return m_position;
}
//=============================================================================
const glm::vec3& DirectionalLight::GetTargetView() const
{
	return m_targetView;
}
//=============================================================================
const glm::vec3& DirectionalLight::GetDirectional() const
{
	return glm::normalize(m_targetView - m_position);
}
//=============================================================================
const glm::mat4& DirectionalLight::GetMatrix()
{
	if (m_needsUpdate)
	{
		glm::vec3 up;
		if (abs(glm::dot(GetDirectional(), glm::vec3(0, 1, 0))) > 0.99f)
		{
			// Если смотришь почти по Y — используем X как up
			up = glm::vec3(1.0f, 0.0f, 0.0f);
		}
		else
		{
			// Иначе — стандартный up
			up = glm::vec3(0.0f, 1.0f, 0.0f);
		}

		glm::mat4 viewMatrix = glm::lookAt(m_position, m_targetView, up);
		glm::mat4 projectionMatrix = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.1f, 100.0f);
		m_viewProj = projectionMatrix * viewMatrix;
		m_needsUpdate = false;
	}

	return m_viewProj;
}
//=============================================================================
const glm::vec3& DirectionalLight::GetColor() const
{
	return m_color;
}
//=============================================================================
float DirectionalLight::GetIntensity() const
{
	return m_intensity;
}
//=============================================================================