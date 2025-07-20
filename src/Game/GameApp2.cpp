#include "stdafx.h"
#include "GameApp2.h"
//�� glRenderer ���������� ���������. ��� ��������� �����
//
//vwa-code - Percentage Closer Filtering(PCF) and Percentage - Closer Soft Shadows(PCSS)
//	��� PCSS
//
//GITechDemo - ����� ������� ��������
//� ���� ��� �������� ������ ��������� - �� ����� ��������. ���� ����� ����� ��������� � ������� ������� ������ - ����� � ����� ���������. ��� ���� ������� �� ���� � �������� - ���� ������������ ��� �����.

//=============================================================================
GameApp2::GameApp2()
	: m_renderWorld(m_world)
{
}
//=============================================================================
EngineCreateInfo GameApp2::GetCreateInfo() const
{
	return {};
}
//=============================================================================
bool GameApp2::OnInit()
{
	OnResize(GetWindowWidth(), GetWindowHeight());

	if (!m_world.Init())
		return false;
	if (!m_renderWorld.Init())
		return false;

	m_camera.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

	return true;
}
//=============================================================================
void GameApp2::OnClose()
{
	m_renderWorld.Close();
	m_world.Close();

	m_colorBuffer = {};
	m_depthBuffer = {};
}
//=============================================================================
void GameApp2::OnUpdate(float deltaTime)
{
	if (Input::IsKeyDown(GLFW_KEY_W)) m_camera.ProcessKeyboard(CameraForward, deltaTime);
	if (Input::IsKeyDown(GLFW_KEY_S)) m_camera.ProcessKeyboard(CameraBackward, deltaTime);
	if (Input::IsKeyDown(GLFW_KEY_A)) m_camera.ProcessKeyboard(CameraLeft, deltaTime);
	if (Input::IsKeyDown(GLFW_KEY_D)) m_camera.ProcessKeyboard(CameraRight, deltaTime);

	if (Input::IsMouseDown(GLFW_MOUSE_BUTTON_RIGHT))
	{
		Input::SetCursorVisible(false);
		m_camera.ProcessMouseMovement(-Input::GetScreenOffset().x, Input::GetScreenOffset().y);
	}
	else if (Input::IsMouseReleased(GLFW_MOUSE_BUTTON_RIGHT))
	{
		Input::SetCursorVisible(true);
	}
}
//=============================================================================
void GameApp2::OnRender()
{
	//-------------------------------------------------------------------------
	// MAIN PASS
	//-------------------------------------------------------------------------
	auto colorAttachment = gl4::RenderColorAttachment{
		.texture = m_colorBuffer.value(),
		.loadOp = gl4::AttachmentLoadOp::Clear,
		.clearValue = { 0.1f, 0.5f, 0.8f, 1.0f },
	};
	auto depthAttachment = gl4::RenderDepthStencilAttachment{
	  .texture = m_depthBuffer.value(),
	  .loadOp = gl4::AttachmentLoadOp::Clear,
	  .clearValue = {.depth = 1.0f},
	};

	gl4::BeginRendering({ .colorAttachments = {&colorAttachment, 1}, .depthAttachment = depthAttachment });
	{
		m_renderWorld.Draw(m_camera);
	}
	gl4::EndRendering();

	//-------------------------------------------------------------------------
	// FINAL PASS
	//-------------------------------------------------------------------------
	gl4::BlitTextureToSwapchain(*m_colorBuffer, {}, {}, m_colorBuffer->Extent(), { GetWindowWidth(), GetWindowHeight(), 1 }, gl4::MagFilter::Nearest);
}
//=============================================================================
void GameApp2::OnImGuiDraw()
{
	DrawFPS();
}
//=============================================================================
void GameApp2::OnResize(uint16_t width, uint16_t height)
{
	m_colorBuffer = gl4::CreateTexture2D({ width, height }, gl4::Format::R8G8B8A8_SRGB, "ColorBuffer");
	m_depthBuffer = gl4::CreateTexture2D({ width, height }, gl4::Format::D32_FLOAT, "DepthBuffer");
}
//=============================================================================
void GameApp2::OnMouseButton(int button, int action, int mods)
{
}
//=============================================================================
void GameApp2::OnMousePos(double x, double y)
{
}
//=============================================================================
void GameApp2::OnScroll(double dx, double dy)
{
}
//=============================================================================
void GameApp2::OnKey(int key, int scanCode, int action, int mods)
{
}
//=============================================================================