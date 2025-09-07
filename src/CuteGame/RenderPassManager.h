#pragma once

#include "Uniforms.h"
#include "TempPass.h"

class RenderPassManager final
{

public:
	bool Init();
	void Close();

	void Resize(uint16_t width, uint16_t height);
	void Final();

	TempPass tempPass;
};