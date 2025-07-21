#include "stdafx.h"
#include "WorldRender.h"
#include "World.h"
//=============================================================================
WorldRender::WorldRender(World& world)
	: m_world(world)
{
}
//=============================================================================
bool WorldRender::Init()
{
	if (!m_shadowPass.Init(&m_world))
		return false;

	if (!m_mainRenderPass.Init(&m_world))
		return false;

	return true;
}
//=============================================================================
void WorldRender::Close()
{

	m_shadowPass.Close();
	m_mainRenderPass.Close();
}
//=============================================================================
void WorldRender::BeginFrame()
{
	setDrawModel(&m_world.m_model1);
	setDrawModel(&m_world.m_model2);
	setDrawModel(&m_world.m_model3);
	setDrawModel(&m_world.m_model4);
}
//=============================================================================
void WorldRender::StartShadowPass()
{
	for (size_t i = 0; i < m_world.GetShadowMap().size(); i++)
	{
		m_shadowPass.Begin(m_world.GetShadowMap()[i]);

		for (size_t j = 0; j < m_currentModel; j++)
		{
			m_shadowPass.DrawModel(*m_models[j]);
		}

		m_shadowPass.End();
	}
}
//=============================================================================
void WorldRender::StartMainRenderPass(Camera& cam, const glm::mat4& proj)
{
	m_mainRenderPass.Begin(cam, proj);

	uint32_t shadowMapIndex = 5;
	for (size_t i = 0; i < m_world.GetShadowMap().size(); i++)
	{
		m_world.GetShadowMap()[i].Bind(shadowMapIndex++, *m_shadowPass.GetLinearSampler());
	}
	for (size_t i = 0; i < m_currentModel; i++)
	{
		m_mainRenderPass.DrawModel(*m_models[i]);
	}
}
//=============================================================================
void WorldRender::EndFrame()
{
	m_currentModel = 0;
}
//=============================================================================
void WorldRender::setDrawModel(GameModel* model)
{
	m_models[m_currentModel++] = model;
}
//=============================================================================