#include "stdafx.h"
#include "RenderPassManager.h"
//=============================================================================
bool RenderPassManager::Init()
{
	if (!tempPass.Init())
		return false;

	if (!shadowMapPass.Init())
		return false;

	return true;
}
//=============================================================================
void RenderPassManager::Close()
{
	shadowMapPass.Close();
	tempPass.Close();
}
//=============================================================================
void RenderPassManager::Resize(uint16_t width, uint16_t height)
{
	tempPass.m_rt.SetSize(width, height);
	shadowMapPass.m_rt.SetSize(width, height);
}
//=============================================================================
void RenderPassManager::Final()
{
	//tempPass.m_rt.BlitToSwapChain();
	shadowMapPass.m_rt.BlitToSwapChain(0);

}
//=============================================================================