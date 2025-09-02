#pragma once

#include "ShadowMapPass.h"

class RenderPassManager final
{
public:
	bool Init();
	void Close();

	ShadowMapPass shadowMapPass;
};