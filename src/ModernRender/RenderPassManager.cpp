#include "stdafx.h"
#include "RenderPassManager.h"
//=============================================================================
bool RenderPassManager::Init()
{
	if (!shadowMapPass.Init())
		return false;

	return true;
}
//=============================================================================
void RenderPassManager::Close()
{
	shadowMapPass.Close();
}
//=============================================================================