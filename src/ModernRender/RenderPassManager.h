#pragma once

#include "Uniforms.h"
#include "Temp2Pass.h"
#include "TempPass.h"

class RenderPassManager final
{

public:
	bool Init();
	void Close();

	void Resize(uint16_t width, uint16_t height);
	void Final();

	Temp2Pass shadowMapPass;
	ForwardPass tempPass;
};