#pragma once

#include "OpenGL4Texture.h"

class TextureManager final
{
public:
	static bool Init();
	static void Close();

	static gl::Texture* GetDefaultTexture2D();

	static gl::Texture* GetTexture(const std::string& name, bool flipVertical = true);

private:
	static inline std::unordered_map<std::string, gl::Texture*> m_texturesMap;
	static inline gl::Texture* m_default2D{ nullptr };
};