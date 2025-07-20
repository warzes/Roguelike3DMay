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
	auto model = m_world.m_model1;
	m_mainRenderPass.SetState(cam, model);
	model.mesh->Bind();
}
//=============================================================================