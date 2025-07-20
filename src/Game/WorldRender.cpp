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
	if (!m_mainRenderPass.Init())
		return false;

	return true;
}
//=============================================================================
void WorldRender::Close()
{
	m_mainRenderPass.Close();
}
//=============================================================================
void WorldRender::Draw(Camera& cam)
{
	//-------------------------------------------------------------------------
	// INIT DATA
	//-------------------------------------------------------------------------
	setDrawModel(&m_world.m_model1);
	setDrawModel(&m_world.m_model2);
	setDrawModel(&m_world.m_model3);
	setDrawModel(&m_world.m_model4);
	setDrawModel(&m_world.m_model5);

	//-------------------------------------------------------------------------
	// MAIN RENDER PASS
	//-------------------------------------------------------------------------
	m_mainRenderPass.Begin(cam);
	for (size_t i = 0; i < m_currentModel; i++)
	{
		m_mainRenderPass.DrawModel(*m_models[i]);
	}

	//-------------------------------------------------------------------------
	// END
	//-------------------------------------------------------------------------
	m_currentModel = 0;
}
//=============================================================================
void WorldRender::setDrawModel(GameModel* model)
{
	m_models[m_currentModel++] = model;
}
//=============================================================================