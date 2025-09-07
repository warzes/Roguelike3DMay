#include "stdafx.h"
#include "Demo001.h"
//=============================================================================
EngineCreateInfo Demo001::GetCreateInfo() const
{
	EngineCreateInfo createInfo{};
	return createInfo;
}
//=============================================================================
bool Demo001::OnInit()
{
	if (!m_renderPassManager.Init())
		return false;

	m_box.Create(GeometryGenerator::CreateBox());
	m_plane.Create(GeometryGenerator::CreatePlane(100.0f, 100.0f, 100.0f, 100.0f));
	m_sphere.Create(GeometryGenerator::CreateSphere());

	auto matrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.002f));
	matrix = glm::rotate(matrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	m_house.Load("ExampleData/mesh/scheune_3ds/scheune.3ds", matrix);

	SceneDataUBO.Init();
	ModelDataUBO.Init();

	m_texture1 = TextureManager::GetTexture("ExampleData/textures/metal.png", true);
	m_texture2 = TextureManager::GetTexture("ExampleData/textures/marble.jpg", true);

	gl::SamplerState sampleDesc;
	sampleDesc.minFilter = gl::MinFilter::Nearest;
	sampleDesc.magFilter = gl::MagFilter::Nearest;
	sampleDesc.addressModeU = gl::AddressMode::Repeat;
	sampleDesc.addressModeV = gl::AddressMode::Repeat;
	m_sampler = gl::Sampler(sampleDesc);

	m_camera.SetPosition(glm::vec3(0.0f, 1.4f, -6.0f));
	m_camera.MovementSpeed = 20.0f;

	OnResize(GetWindowWidth(), GetWindowHeight());

	return true;
}
//=============================================================================
void Demo001::OnClose()
{
	m_renderPassManager.Close();
	m_box.Free();
	m_plane.Free();
	m_sphere.Free();
	m_house.Free();
	SceneDataUBO.Close();
	ModelDataUBO.Close();
	m_sampler = {};
	m_texture1 = nullptr;
	m_texture2 = nullptr;
}
//=============================================================================
void Demo001::OnUpdate([[maybe_unused]] float deltaTime)
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

	SceneDataUBO->viewMatrix = m_camera.GetViewMatrix();
	SceneDataUBO->projectionMatrix = glm::perspective(glm::radians(65.0f), GetWindowAspect(), 0.1f, 1000.0f);
}
//=============================================================================
void Demo001::OnRender()
{
	m_renderPassManager.tempPass.Begin({ 0.1f, 0.5f, 0.8f });
	{
		SceneDataUBO.Bind(0);
		sceneDraw();
	}
	m_renderPassManager.tempPass.End();

	m_renderPassManager.Final();
}
//=============================================================================
void Demo001::OnImGuiDraw()
{
	DrawFPS();
}
//=============================================================================
void Demo001::OnResize(uint16_t width, uint16_t height)
{
	m_renderPassManager.Resize(width, height);
}
//=============================================================================
void Demo001::OnMouseButton([[maybe_unused]] int button, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================
void Demo001::OnMousePos([[maybe_unused]] double x, [[maybe_unused]] double y)
{
}
//=============================================================================
void Demo001::OnScroll([[maybe_unused]] double dx, [[maybe_unused]] double dy)
{
}
//=============================================================================
void Demo001::OnKey([[maybe_unused]] int key, [[maybe_unused]] int scanCode, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
}
//=============================================================================
void Demo001::sceneDraw()
{
	// плоскость
	{
		ModelDataUBO->modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		ModelDataUBO.Bind(1);

		gl::Cmd::BindSampledImage(0, *m_texture1, *m_sampler);
		m_plane.Draw(std::nullopt);
	}

	// куб
	{
		ModelDataUBO->modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 1.0f, 0.0f));
		ModelDataUBO.Bind(1);

		gl::Cmd::BindSampledImage(0, *m_texture2, *m_sampler);
		m_box.Draw(std::nullopt);
	}

	// Сфера
	{
		ModelDataUBO->modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 5.0f, 0.0f));
		ModelDataUBO.Bind(1);

		gl::Cmd::BindSampledImage(0, *m_texture2, *m_sampler);
		m_sphere.Draw(std::nullopt);
	}

	// Дом
	{
		std::vector<glm::vec3> housePositions
		{
			glm::vec3(0.0f, 0.0f, -10.0f),
			glm::vec3(-20.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 10.0f),
			glm::vec3(20.0f, 0.0f, 0.0f),
			glm::vec3(5.0f, 0.0f, 0.0f),
		};
		for (const auto& housePosition : housePositions)
		{
			ModelDataUBO->modelMatrix = glm::translate(glm::mat4(1.0f), housePosition);
			ModelDataUBO.Bind(1);

			m_house.Draw(m_sampler);
		}
	}
}
//=============================================================================