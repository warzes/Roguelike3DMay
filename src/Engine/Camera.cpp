#include "stdafx.h"
#include "Camera.h"
//=============================================================================
Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
	: Position(position)
	, Front(glm::vec3(0.0f, 0.0f, -1.0f))
	, Up(glm::vec3(0.f, 1.f, 0.f))
	, Right(glm::vec3(1.f, 0.f, 0.f))
	, WorldUp(up)
	, Yaw(yaw)
	, Pitch(pitch)
	, MovementSpeed(CAMERA_SPEED)
	, MouseSensitivity(CAMERA_SENSITIVITY)
	, m_viewMatrix(glm::mat4(1.f))
{
	updateInternal();
}
//=============================================================================
glm::mat4 Camera::GetViewMatrix() const
{
	return m_viewMatrix;
}
//=============================================================================
void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
{
	const float velocity = MovementSpeed * deltaTime;
	if (direction == CameraForward)
	{
		Position += Front * velocity;
	}
	else if (direction == CameraBackward)
	{
		Position -= Front * velocity;
	}
	else if (direction == CameraLeft)
	{
		Position -= Right * velocity;
	}
	else if (direction == CameraRight)
	{
		Position += Right * velocity;
	}

	updateInternal();
}
//=============================================================================
void Camera::ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch)
{
	xOffset *= MouseSensitivity;
	yOffset *= MouseSensitivity;

	Yaw += xOffset;
	Pitch += yOffset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
	{
		if (Pitch > 89.0f)
		{
			Pitch = 89.0f;
		}
		else if (Pitch < -89.0f)
		{
			Pitch = -89.0f;
		}
	}

	// Update vectors and matrices
	updateInternal();
}
//=============================================================================
void Camera::updateInternal()
{
	// Recalculate front vector
	Front = glm::normalize(
		glm::vec3
		(
			cos(glm::radians(Yaw)) * cos(glm::radians(Pitch)), // x
			sin(glm::radians(Pitch)), // y
			sin(glm::radians(Yaw)) * cos(glm::radians(Pitch)) // z
		)
	);

	// Recalculate right and up vector
	Right = glm::normalize(glm::cross(Front, WorldUp));
	Up = glm::normalize(glm::cross(Right, Front));

	// Matrices
	m_viewMatrix = glm::lookAt(Position, Position + Front, Up);
}
//=============================================================================