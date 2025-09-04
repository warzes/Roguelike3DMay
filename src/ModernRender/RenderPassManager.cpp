#include "stdafx.h"
#include "RenderPassManager.h"
/*
Normal maps, specular maps, roughness, metalness, и другие нецветовые данные — не должны быть в sRGB формате.
→ Используйте линейные (non-sRGB) форматы: GL_RGBA8, GL_RG8, GL_R8, и т.п.
В G-Buffer:
Albedo (diffuse) — GL_SRGB8_ALPHA8 или GL_R8G8B8A8_SRGB ✅ (если это цвет)
Normal — GL_RGBA16F, GL_RG16F, или GL_RGB10_A2 ❌ не sRGB
Specular / Roughness / Metalness / Emission — GL_RGBA8, GL_RG8, GL_R8, и т.п. ❌ не sRGB
Depth/Stencil — отдельно как GL_DEPTH_COMPONENT32F или GL_DEPTH24_STENCIL8
*/
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
	tempPass.m_rt.BlitToSwapChain();
	//shadowMapPass.m_rt.BlitToSwapChain(0);
}
//=============================================================================