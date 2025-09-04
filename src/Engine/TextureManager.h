#pragma once

#include "OpenGL4Texture.h"

class TextureManager final
{
public:
	// пока sRGB тут, надо проверить как правильно
	static bool Init(bool sRGB);
	static void Close();

	static gl::Texture* GetDefaultDiffuse2D();
	static gl::Texture* GetDefaultNormal2D();
	static gl::Texture* GetDefaultSpecular2D();

	static gl::Texture* GetTexture(const std::string& name, bool flipVertical = false);
};