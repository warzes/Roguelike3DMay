#include "stdafx.h"
#include "DirectionalLight.h"
//=============================================================================
glm::mat4 DirectionalLight::GetMatrix() const
{
	glm::vec3 up;
	if (abs(glm::dot(GetDirectional(), glm::vec3(0, 1, 0))) > 0.99f)
	{
		// Если смотришь почти по Y — используем X как up
		up = glm::vec3(1, 0, 0);
	}
	else {
		// Иначе — стандартный up
		up = glm::vec3(0, 1, 0);
	}


	glm::mat4 viewMatrix = glm::lookAt(position, targetView, up);
	glm::mat4 projectionMatrix = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 1.0f, 100.0f);
	return projectionMatrix * viewMatrix;
}
//=============================================================================
glm::vec3 DirectionalLight::GetDirectional() const
{
	return glm::normalize(targetView - position);
}
//=============================================================================