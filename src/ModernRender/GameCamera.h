#pragma once

#include "GameObject.h"

enum class GameCameraProjectMode : uint8_t
{
	Perspective,
	Ortho,
};

class GameCamera final : public GameObject
{
public:
	GameCamera();

	void SetPerspectiveCamera(float fov, float nearClip, float farClip);
	void SetPerspectiveCamera(float fov, float nearClip, float farClip, int size);

	void SetNearClip(float nearClip);
	float GetNearClip() const;
	void SetFarClip(float farClip);
	float GetFarClip() const;
	void SetFov(float fov);
	float GetFov() const;
	void SetOrthoSize(float size);
	glm::vec3 GetCameraPara() const;
	void SetOrthoCamera(float size, float nearClip, float farClip);
	void SetOrthoCamera(float size, float nearClip, float farClip, float sideSize);

	void UpdateProjectionMatrix();
	void UpdateViewMatrix();
	const glm::mat4& GetViewMatrix() const;
	const glm::mat4& GetProjectionMatrix() const;

private:
	GameCameraProjectMode m_curCameraMode;
	Camera    m_camera;
	glm::mat4 m_projectionMatrix{ glm::mat4(1.0f) };

	float m_fov{ 60.0f };      // vertical direction
	float m_orthoSize{ 5.0f }; // half of height
	float m_nearClip{ 0.01f };
	float m_farClip{ 1000.0f };
	float m_wInPixel{ 1600.0f };
	float m_hInPixel{ 900.0f };

	glm::vec3 m_nearClipPtLT{ 0.0f };
	glm::vec3 m_nearClipPtLB{ 0.0f };
	glm::vec3 m_nearClipPtRT{ 0.0f };
	glm::vec3 m_nearClipPtRB{ 0.0f };

	glm::vec3 m_groundPtLT{ 0.0f };
	glm::vec3 m_groundPtLB{ 0.0f };
	glm::vec3 m_groundPtRT{ 0.0f };
	glm::vec3 m_groundPtRB{ 0.0f };
};