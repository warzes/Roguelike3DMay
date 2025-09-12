#pragma once

#include "OpenGL4Texture.h"

class TextureManager final
{
public:
	static bool Init();
	static void Close();

	static gl::Texture* GetDefaultDiffuse2D();
	static gl::Texture* GetDefaultNormal2D();
	static gl::Texture* GetDefaultSpecular2D();

	static gl::Texture* GetTexture(const std::string& name, gl::ColorSpace colorSpace = gl::ColorSpace::None, bool flipVertical = false);
};