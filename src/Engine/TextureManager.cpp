#include "stdafx.h"
#include "TextureManager.h"
#include "Log.h"
//=============================================================================
bool TextureManager::Init()
{
	return true;
}
//=============================================================================
void TextureManager::Close()
{
	for (auto& it : m_textures)
	{
		delete it.second;
	}
	m_textures.clear();
}
//=============================================================================
gl4::Texture* TextureManager::GetTexture(const std::string& name, bool flipVertical)
{
	auto it = m_textures.find(name);
	if (it != m_textures.end())
	{
		return it->second;
	}
	else
	{
		bool hasTex = std::filesystem::exists(name) && std::filesystem::is_regular_file(name);
		if (hasTex == false)
		{
			Error("Failed to load texture " + name);
			return nullptr;
		}

		stbi_set_flip_vertically_on_load(flipVertical);

		int imgW, imgH, nrChannels;
		auto pixels = stbi_loadf(name.c_str(), &imgW, &imgH, &nrChannels, 0);
		if (!pixels)
		{
			Error("Failed to load texture " + name);
			return nullptr;
		}
		gl4::Format imgFormat{ gl4::Format::R8G8B8_UNORM };
		if (nrChannels == 1) imgFormat = gl4::Format::R8_UNORM;
		else if (nrChannels == 3) imgFormat = gl4::Format::R8G8B8_UNORM;
		else if (nrChannels == 4) imgFormat = gl4::Format::R8G8B8A8_UNORM;

		const gl4::TextureCreateInfo createInfo{
		  .imageType   = gl4::ImageType::Tex2D,
		  .format      = imgFormat,
		  .extent      = {static_cast<uint32_t>(imgW), static_cast<uint32_t>(imgH), 1},
		  .mipLevels   = 1,
		  .arrayLayers = 1,
		  .sampleCount = gl4::SampleCount::Samples1,
		};
		m_textures[name] = new gl4::Texture(createInfo, name);
		gl4::Texture& texture = *m_textures[name];

		gl4::UploadFormat texFormat{ gl4::UploadFormat::RGB };
		if (nrChannels == 1) texFormat = gl4::UploadFormat::R;
		else if (nrChannels == 3) texFormat = gl4::UploadFormat::RGB;
		else if (nrChannels == 4) texFormat = gl4::UploadFormat::RGBA;

		texture.UpdateImage({
		  .extent = {static_cast<uint32_t>(imgW), static_cast<uint32_t>(imgH)},
		  .format = texFormat,
		  .type   = gl4::UploadType::FLOAT,
		  .pixels = pixels,
			});
		stbi_image_free(pixels);

		return &texture;
	}
}
//=============================================================================