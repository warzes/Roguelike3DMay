#include "stdafx.h"
#include "GameSceneManager.h"
//=============================================================================
bool GameSceneManager::Init()
{
	if (!m_modelManager.Init())
		return false;

	m_sceneUniformUbo = gl4::TypedBuffer<sceneUBO::SceneUniforms>(gl4::BufferStorageFlag::DynamicStorage);

	m_lights.resize(4);

	m_lights[0].diffuseColor = glm::vec3(1.0f, 0.1f, 0.2f);  // Red
	m_lights[0].position = glm::vec3(4.0, 5.0, -3.0);

	m_lights[1].diffuseColor = glm::vec3(0.0f, 1.0f, 0.0f);  // Green
	m_lights[1].position = glm::vec3(3.0f, 1.0f, 3.0f);

	m_lights[2].diffuseColor = glm::vec3(0.0f, 0.0f, 1.0f);  // Blue
	m_lights[2].position = glm::vec3(-3.0f, 1.0f, -3.0f);

	m_lights[3].diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);  // White
	m_lights[3].position = glm::vec3(3.0f, 1.0f, -3.0f);

	m_lightSSBO.emplace(std::span(m_lights), gl4::BufferStorageFlag::DynamicStorage);

	return true;
}
//=============================================================================
void GameSceneManager::Close()
{
	m_lightSSBO = {};
	m_modelManager.Close();
}
//=============================================================================
void GameSceneManager::Update(Camera& cam)
{
	m_modelManager.Update(cam);

	sceneUBO::SceneUniforms sceneUbo;
	sceneUbo.NumLight = 4;
	m_sceneUniformUbo->UpdateData(sceneUbo);
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
	gl4::Cmd::BindStorageBuffer(0, *m_lightSSBO);
	m_modelManager.Draw();
}
//=============================================================================