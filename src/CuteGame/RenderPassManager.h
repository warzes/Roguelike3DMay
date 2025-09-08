#pragma once

#include "Uniforms.h"
#include "ForwardPass.h"
#include "ShadowMapPass.h"
#include "DepthPass.h"

class RenderPassManager final
{

public:
	bool Init();
	void Close();

	void Resize(uint16_t width, uint16_t height);
	void Final();

	DepthPass depthPass;

	Temp2Pass shadowMapPass;
	ForwardPass tempPass;
};