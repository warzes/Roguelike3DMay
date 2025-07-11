#pragma once

enum CameraMovement
{
	CameraForward,
	CameraBackward,
	CameraLeft,
	CameraRight,
};

// Default camera values
constexpr float CAMERA_YAW = 90.0f;
constexpr float CAMERA_PITCH = 0.0f;
constexpr float CAMERA_SPEED = 2.5f;
constexpr float CAMERA_SENSITIVITY = 0.1f;

class Camera final
{
public:
	Camera(
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
		float yaw = CAMERA_YAW,
		float pitch = CAMERA_PITCH);

	[[nodiscard]] glm::mat4 GetViewMatrix() const;

	void ProcessKeyboard(CameraMovement direction, float deltaTime);
	void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);

	void SetPosition(const glm::vec3& position);

	// Attributes
	glm::vec3 Position{};
	glm::vec3 Front{};
	glm::vec3 Up{};
	glm::vec3 Right{};
	glm::vec3 WorldUp{};

	// Euler Angles
	float Yaw{};
	float Pitch{};

	// Options
	float MovementSpeed{};
	float MouseSensitivity{};

private:
	void updateInternal();

	glm::mat4 m_viewMatrix{};
};