#include "stdafx.h"
#include "ShadowMapPass.h"
//=============================================================================
bool ShadowMapPass::Init()
{
	m_rt.SetName("ShadowMapPassColor", "ShadowMapPassDepth");
	m_rt.SetSize(1024, 1024);
	return true;
}
//=============================================================================
void ShadowMapPass::Close()
{
	m_rt.Close();
}
//=============================================================================
void ShadowMapPass::Begin()
{
	m_rt.Begin({ 0.0f, 0.0f, 0.0f });
}
//=============================================================================
void ShadowMapPass::End()
{
	m_rt.End();
}
//=============================================================================