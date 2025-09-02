#include "stdafx.h"
#include "GameCamera.h"
//=============================================================================
GameCamera::GameCamera()
{
	m_type = GameObjectType::Camera;
}
//=============================================================================
void GameCamera::SetPerspectiveCamera(float fov, float nearClip, float farClip)
{
	m_fov = fov;
	m_nearClip = nearClip;
	m_farClip = farClip;
	m_wInPixel = GetWindowWidth();
	m_hInPixel = GetWindowHeight();
	m_curCameraMode = GameCameraProjectMode::Perspective;

	UpdateProjectionMatrix();
}
//=============================================================================
void GameCamera::SetPerspectiveCamera(float fov, float nearClip, float farClip, int size)
{
	m_fov = fov;
	m_nearClip = nearClip;
	m_farClip = farClip;
	m_wInPixel = size;
	m_hInPixel = size;
	m_curCameraMode = GameCameraProjectMode::Perspective;

	UpdateProjectionMatrix();
}
//=============================================================================
void GameCamera::SetNearClip(float nearClip)
{
	m_nearClip = nearClip;
	UpdateProjectionMatrix();
}
//=============================================================================
float GameCamera::GetNearClip() const
{
	return m_nearClip;
}
//=============================================================================
void GameCamera::SetFarClip(float farClip)
{
	m_farClip = farClip;
	UpdateProjectionMatrix();
}
//=============================================================================
float GameCamera::GetFarClip() const
{
	return m_farClip;
}
//=============================================================================
void GameCamera::SetFov(float fov)
{
	m_fov = fov;
	UpdateProjectionMatrix();
}
//=============================================================================
float GameCamera::GetFov() const
{
	return m_fov;
}
//=============================================================================
void GameCamera::SetOrthoSize(float size)
{
	m_orthoSize = size;
	UpdateProjectionMatrix();
}
//=============================================================================
glm::vec3 GameCamera::GetCameraPara() const
{
	return glm::vec3(m_fov, m_nearClip, m_farClip);
}
//=============================================================================
void GameCamera::SetOrthoCamera(float size, float nearClip, float farClip)
{
	m_orthoSize = size;
	m_nearClip = nearClip;
	m_farClip = farClip;
	m_wInPixel = GetWindowWidth();
	m_hInPixel = GetWindowHeight();
	m_curCameraMode = GameCameraProjectMode::Ortho;

	UpdateProjectionMatrix();
}
//=============================================================================
void GameCamera::SetOrthoCamera(float size, float nearClip, float farClip, float sideSize)
{
	m_orthoSize = size;
	m_nearClip = nearClip;
	m_farClip = farClip;
	m_wInPixel = sideSize;
	m_hInPixel = sideSize;
	m_curCameraMode = GameCameraProjectMode::Ortho;

	UpdateProjectionMatrix();
}
//=============================================================================
void GameCamera::UpdateProjectionMatrix()
{
	if (m_curCameraMode == GameCameraProjectMode::Perspective)
	{
		float aspect = m_hInPixel / m_wInPixel;
		m_projectionMatrix = glm::perspective(m_fov, aspect, m_nearClip, m_farClip);
	}
	else if (m_curCameraMode == GameCameraProjectMode::Ortho)
	{
		m_projectionMatrix = glm::ortho(0, 0, 1, 1); // TODO: доделать
	}
}
//=============================================================================
void GameCamera::UpdateViewMatrix()
{
	m_camera.SetPosition(m_position);
}
//=============================================================================
const glm::mat4& GameCamera::GetViewMatrix() const
{
	return m_camera.GetViewMatrix();
}
//=============================================================================
const glm::mat4& GameCamera::GetProjectionMatrix() const
{
	return m_projectionMatrix;
}
//=============================================================================