#pragma once

#include "OpenGL4Texture.h"

class TextureManager final
{
public:
	static bool Init();
	static void Close();

	static gl4::Texture* GetTexture(const std::string& name);

private:
	static inline std::unordered_map<std::string, gl4::Texture*> m_textures;
};