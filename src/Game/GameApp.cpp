#include "stdafx.h"
#include "GameApp.h"
//=============================================================================
EngineCreateInfo GameApp::GetCreateInfo() const
{
	return {};
}
//=============================================================================
bool GameApp::OnInit()
{
	if (!m_graphics.Init(this))
		return false;

	m_graphics.Resize(GetWindowWidth(), GetWindowHeight());

	std::vector<MeshVertex> vertices = {
		// positions            // normals            // texcoords
		{{-50.0f, -0.5f,  50.0f},  {0.0f, 1.0f, 0.0f},  { 0.0f,  0.0f}},
		{{-50.0f, -0.5f, -50.0f},  {0.0f, 1.0f, 0.0f},  { 0.0f, 50.0f}},
		{{ 50.0f, -0.5f,  50.0f},  {0.0f, 1.0f, 0.0f},  {50.0f,  0.0f}},
		{{ 50.0f, -0.5f,  50.0f},  {0.0f, 1.0f, 0.0f},  {50.0f,  0.0f}},
		{{-50.0f, -0.5f, -50.0f},  {0.0f, 1.0f, 0.0f},  { 0.0f, 50.0f}},
		{{ 50.0f, -0.5f, -50.0f},  {0.0f, 1.0f, 0.0f},  {50.0f, 50.0f}}
	};
	std::vector<uint32_t> iv = { 0, 1, 2, 3, 4, 5 };

	m_model1.mesh = LoadDataMesh(vertices, iv);
	m_model1.diffuse = TextureManager::GetTexture("CoreData/textures/colorful.png");
	m_model1.textureFilter = gl4::MagFilter::Nearest;

	m_model2.mesh = LoadAssimpMesh("CoreData/mesh/Cube/Cube.gltf");
	m_model2.diffuse = TextureManager::GetTexture("CoreData/textures/colorful.png");
	m_model2.textureFilter = gl4::MagFilter::Nearest;

	m_model3.mesh = LoadAssimpMesh("ExampleData/mesh/stall/stall.obj");
	m_model3.diffuse = TextureManager::GetTexture("ExampleData/mesh/stall/stallTexture.png", false);
	m_model3.textureFilter = gl4::MagFilter::Linear;
	m_model3.scale = glm::vec3(0.3f);
	m_model3.position = glm::vec3(4.0f, -0.6f, 0.0f);

	m_camera.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

	return true;
}
//=============================================================================
void GameApp::OnClose()
{
	delete m_model1.mesh;
	delete m_model2.mesh;
	delete m_model3.mesh;
	m_graphics.Close();
}
//=============================================================================
void GameApp::OnUpdate(float deltaTime)
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

	m_graphics.Update(deltaTime);
}
//=============================================================================
void GameApp::OnRender()
{
	m_graphics.SetModel(&m_model1);
	m_graphics.SetModel(&m_model2);
	m_graphics.SetModel(&m_model3);

	m_graphics.Render();
}
//=============================================================================
void GameApp::OnImGuiDraw()
{
	DrawFPS();
}
//=============================================================================
void GameApp::OnResize(uint16_t width, uint16_t height)
{
	m_graphics.Resize(width, height);
}
//=============================================================================
void GameApp::OnMouseButton(int button, int action, int mods)
{
}
//=============================================================================
void GameApp::OnMousePos(double x, double y)
{
}
//=============================================================================
void GameApp::OnScroll(double dx, double dy)
{
}
//=============================================================================
void GameApp::OnKey(int key, int scanCode, int action, int mods)
{
}
//=============================================================================