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
	OnResize(GetWindowWidth(), GetWindowHeight());

	std::vector<MeshVertex> vertices = {
		// positions                                // normals            // texcoords
		{{-50.0f, -0.5f,  50.0f}, glm::vec3(1.0f), {0.0f, 1.0f, 0.0f},  { 0.0f,  0.0f}},
		{{-50.0f, -0.5f, -50.0f}, glm::vec3(1.0f), {0.0f, 1.0f, 0.0f},  { 0.0f, 50.0f}},
		{{ 50.0f, -0.5f,  50.0f}, glm::vec3(1.0f), {0.0f, 1.0f, 0.0f},  {50.0f,  0.0f}},
		{{ 50.0f, -0.5f,  50.0f}, glm::vec3(1.0f), {0.0f, 1.0f, 0.0f},  {50.0f,  0.0f}},
		{{-50.0f, -0.5f, -50.0f}, glm::vec3(1.0f), {0.0f, 1.0f, 0.0f},  { 0.0f, 50.0f}},
		{{ 50.0f, -0.5f, -50.0f}, glm::vec3(1.0f), {0.0f, 1.0f, 0.0f},  {50.0f, 50.0f}}
	};
	std::vector<uint32_t> iv = { 0, 1, 2, 3, 4, 5 };

	m_model1.mesh = LoadDataMesh(vertices, iv);
	m_model1.textureFilter = gl::MagFilter::Nearest;
	m_model1.material.diffuseTexture = TextureManager::GetTexture("CoreData/textures/colorful.png", gl::ColorSpace::sRGB);
	m_model1.material.normalTexture = TextureManager::GetTexture("CoreData/textures/normal01.tga");

	m_model2.mesh = LoadAssimpMesh("CoreData/mesh/Cube/Cube.gltf");
	m_model2.textureFilter = gl::MagFilter::Nearest;
	m_model2.material.diffuseTexture = TextureManager::GetTexture("CoreData/textures/colorful.png", gl::ColorSpace::sRGB);
	m_model2.material.normalTexture = TextureManager::GetTexture("CoreData/textures/normal01.tga");

	m_model3.mesh = LoadAssimpMesh("ExampleData/mesh/stall/stall.obj");
	m_model3.scale = glm::vec3(0.3f);
	m_model3.position = glm::vec3(4.0f, -0.6f, 0.0f);
	m_model3.textureFilter = gl::MagFilter::Linear;
	m_model3.material.diffuseTexture = TextureManager::GetTexture("ExampleData/mesh/stall/stallTexture.png", gl::ColorSpace::sRGB, false);
	m_model3.material.normalTexture = TextureManager::GetTexture("CoreData/textures/normal01.tga");

	m_camera.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

	if (!m_scene.Init())
		return false;

	return true;
}
//=============================================================================
void GameApp::OnClose()
{
	delete m_model1.mesh;
	delete m_model2.mesh;
	delete m_model3.mesh;
	m_scene.Close();
	m_colorBuffer = {};
	m_depthBuffer = {};
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
		m_camera.ProcessMouseMovement(Input::GetCursorOffset().x, Input::GetCursorOffset().y);
	}
	else if (Input::IsMouseReleased(GLFW_MOUSE_BUTTON_RIGHT))
	{
		Input::SetCursorVisible(true);
	}

	m_scene.Update();
}
//=============================================================================
void GameApp::OnRender()
{
	m_scene.SetModel(&m_model1);
	m_scene.SetModel(&m_model2);
	m_scene.SetModel(&m_model3);

	m_scene.DrawInDepth(m_camera);

	auto colorAttachment = gl::RenderColorAttachment{
		.texture = m_colorBuffer.value(),
		.loadOp = gl::AttachmentLoadOp::Clear,
		.clearValue = { 0.1f, 0.5f, 0.8f, 1.0f },
	};
	auto depthAttachment = gl::RenderDepthStencilAttachment{
	  .texture = m_depthBuffer.value(),
	  .loadOp = gl::AttachmentLoadOp::Clear,
	  .clearValue = {.depth = 1.0f},
	};

	gl::BeginRendering({ .colorAttachments = {&colorAttachment, 1}, .depthAttachment = depthAttachment });
	{
		m_scene.Draw(m_camera);
	}
	gl::EndRendering();

	gl::BlitTextureToSwapChain(*m_colorBuffer, {}, {}, m_colorBuffer->GetExtent(), { GetWindowWidth(), GetWindowHeight(), 1 }, gl::MagFilter::Nearest);
}
//=============================================================================
void GameApp::OnImGuiDraw()
{
	DrawFPS();
}
//=============================================================================
void GameApp::OnResize(uint16_t width, uint16_t height)
{
	m_colorBuffer = gl::CreateTexture2D({ width, height }, gl::Format::R8G8B8A8_SRGB, "ColorBuffer");
	m_depthBuffer = gl::CreateTexture2D({ width, height }, gl::Format::D32_FLOAT, "DepthBuffer");
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