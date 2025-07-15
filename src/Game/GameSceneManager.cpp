#include "stdafx.h"
#include "GameSceneManager.h"
//=============================================================================
bool GameSceneManager::Init()
{
	if (!m_modelManager.Init())
		return false;

	m_sceneUniformUbo = gl4::TypedBuffer<sceneUBO::SceneUniforms>(gl4::BufferStorageFlag::DynamicStorage);

	m_directionalLights.resize(4);

	m_directionalLights[0].position = glm::vec3(1);
	m_directionalLights[0].color = glm::vec3(1);

	m_pointLights.resize(4);

	m_pointLights[0].position = glm::vec3(2, 0, 2);
	m_pointLights[0].color = glm::vec3(1, 0, 0);

	//m_directionalLights[0].color = glm::vec3(1.0f, 0.1f, 0.2f);  // Red
	//m_directionalLights[0].position = glm::vec3(4.0, 5.0, -3.0);

	//m_directionalLights[1].color = glm::vec3(0.0f, 1.0f, 0.0f);  // Green
	//m_directionalLights[1].position = glm::vec3(3.0f, 1.0f, 3.0f);

	//m_directionalLights[2].color = glm::vec3(0.0f, 0.0f, 1.0f);  // Blue
	//m_directionalLights[2].position = glm::vec3(-3.0f, 1.0f, -3.0f);

	//m_directionalLights[3].color = glm::vec3(1.0f, 1.0f, 1.0f);  // White
	//m_directionalLights[3].position = glm::vec3(3.0f, 1.0f, -3.0f);



	m_directionalLightSSBO.emplace(std::span(m_directionalLights), gl4::BufferStorageFlag::DynamicStorage);
	m_pointLightSSBO.emplace(std::span(m_pointLights), gl4::BufferStorageFlag::DynamicStorage);

	return true;
}
//=============================================================================
void GameSceneManager::Close()
{
	m_directionalLightSSBO = {};
	m_pointLightSSBO = {};
	m_modelManager.Close();
}
//=============================================================================
void GameSceneManager::Update(Camera& cam)
{
	m_modelManager.Update(cam);

	sceneUBO::SceneUniforms sceneUbo;
	sceneUbo.NumDirectionalLight = 1;
	sceneUbo.NumPointLight = 1;
	sceneUbo.CameraPos = cam.Position;
	m_sceneUniformUbo->UpdateData(sceneUbo);

	m_directionalLightSSBO->UpdateData(std::span(m_directionalLights));
	m_pointLightSSBO->UpdateData(std::span(m_pointLights));
}
//=============================================================================
void GameSceneManager::SetModel(GameModel* model)
{
	m_modelManager.SetModel(model);
}
//=============================================================================
void GameSceneManager::Draw()
{
	gl4::Cmd::BindUniformBuffer(1, m_sceneUniformUbo.value());
	gl4::Cmd::BindStorageBuffer(0, *m_directionalLightSSBO);
	gl4::Cmd::BindStorageBuffer(1, *m_pointLightSSBO);
	m_modelManager.Draw();
}
//=============================================================================