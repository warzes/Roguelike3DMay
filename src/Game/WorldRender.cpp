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
	if (!m_shadowPass.Init())
		return false;

	if (!m_mainRenderPass.Init(m_world.m_lights))
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
void WorldRender::StartShadowPass(Camera& cam, const glm::mat4& proj)
{
	m_shadowPass.Begin();
	for (size_t i = 0; i < m_currentModel; i++)
	{
		m_shadowPass.DrawModel(*m_models[i]);
	}
	m_shadowPass.End();
}
//=============================================================================
void WorldRender::StartMainRenderPass(Camera& cam, const glm::mat4& proj)
{
	m_mainRenderPass.Begin(m_world.m_lights, cam, proj);
	m_shadowPass.BindShadowMap(5);
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