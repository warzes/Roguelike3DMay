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

	static gl::Texture* GetTexture(const std::string& name, bool flipVertical = false);

private:
	static inline std::unordered_map<std::string, gl::Texture*> m_texturesMap;
	static inline gl::Texture* m_defaultDiffuse2D{ nullptr };
	static inline gl::Texture* m_defaultNormal2D{ nullptr };
	static inline gl::Texture* m_defaultSpecular2D{ nullptr };
};