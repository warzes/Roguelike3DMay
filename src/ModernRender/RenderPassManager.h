#pragma once

#include "Uniforms.h"
#include "ShadowMapPass.h"
#include "TempPass.h"

class RenderPassManager final
{
public:
	bool Init();
	void Close();

	ShadowMapPass shadowMapPass;

	TempPass tempPass;
};