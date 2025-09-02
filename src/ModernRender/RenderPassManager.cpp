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