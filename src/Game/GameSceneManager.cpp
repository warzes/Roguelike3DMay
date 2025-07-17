#include "stdafx.h"
#include "GameSceneManager.h"

struct Attenuation
{
	float constant;
	float linear;
	float quadratic;
};

Attenuation calculateAttenuation(float radius, float targetIntensity)
{
	Attenuation att;
	att.constant = 1.0f;
	att.linear = 0.35f;

	float totalAttenuation = 1.0f / targetIntensity;
	att.quadratic = (totalAttenuation - att.constant - att.linear * radius) / (radius * radius);
	return att;
}
//=============================================================================
bool GameSceneManager::Init()
{
	if (!m_modelManager.Init())
		return false;

	if (!m_shadowPassMgr.Init())
		return false;

	m_sceneUniformUbo = gl4::TypedBuffer<sceneUBO::SceneUniforms>(gl4::BufferStorageFlag::DynamicStorage);

	m_lights.resize(3);

	Attenuation att = calculateAttenuation(23.0f, 0.01f);

	m_lights[0].position = glm::vec3(1);
	m_lights[0].color = glm::vec3(1);
	m_lights[0].type = DirectionalLight;

	m_lights[1].position = glm::vec3(5, 0, 0);
	m_lights[1].color = glm::vec3(1, 0, 0);
	m_lights[1].type = PointLight;
	m_lights[1].constant = att.constant;
	m_lights[1].linear = att.linear;
	m_lights[1].quadratic = att.quadratic;

	m_lights[2].position = glm::vec3(-5, 0, 0);
	m_lights[2].color = glm::vec3(0, 1, 0);
	m_lights[2].type = PointLight;
	m_lights[2].constant = 1.0f;
	m_lights[2].linear = 0.35f;
	m_lights[2].quadratic = 0.44f;

	m_lightSSBO.emplace(std::span(m_lights), gl4::BufferStorageFlag::DynamicStorage);

	return true;
}
//=============================================================================
void GameSceneManager::Close()
{
	m_lightSSBO = {};
	m_shadowPassMgr.Close();
	m_modelManager.Close();
}
//=============================================================================
void GameSceneManager::Update()
{
	m_modelManager.Update();
	m_lightSSBO->UpdateData(std::span(m_lights));
}
//=============================================================================
void GameSceneManager::SetModel(GameModel* model)
{
	m_modelManager.SetModel(model);
}
//=============================================================================
void GameSceneManager::Draw(Camera& cam)
{
	sceneUBO::SceneUniforms sceneUbo;
	sceneUbo.CameraPos = cam.Position;
	sceneUbo.NumLight = 3;
	m_sceneUniformUbo->UpdateData(sceneUbo);

	gl4::Cmd::BindUniformBuffer(1, m_sceneUniformUbo.value());
	gl4::Cmd::BindStorageBuffer(0, *m_lightSSBO);
	m_modelManager.Draw(cam);
}
//=============================================================================
void GameSceneManager::DrawInDepth(Camera& cam)
{
	gl4::BeginRendering(*m_shadowPassMgr.GetShadowPass().viewport);
	{
		m_modelManager.DrawInDepth(cam);
	}
	gl4::EndRendering();
}
//=============================================================================