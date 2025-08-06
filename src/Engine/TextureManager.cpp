#include "stdafx.h"
#include "TextureManager.h"
#include "Log.h"
#include "FileUtils.h"
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
gl::Texture* TextureManager::GetTexture(const std::string& name, bool flipVertical)
{
	auto it = m_textures.find(name);
	if (it != m_textures.end())
	{
		return it->second;
	}
	else
	{
		bool hasTex = io::Exists(name);
		if (hasTex == false)
		{
			Error("Failed to load texture " + name);
			return nullptr;
		}

		stbi_set_flip_vertically_on_load(flipVertical);

		int imgW, imgH, nrChannels;
		auto pixels = stbi_loadf(name.c_str(), &imgW, &imgH, &nrChannels, 0);
		if (!pixels || nrChannels < 1 || nrChannels > 4 || imgW < 0 || imgH < 0)
		{
			Error("Failed to load texture " + name);
			return nullptr;
		}
		gl::Format imgFormat{ gl::Format::R8G8B8_UNORM };
		if (nrChannels == 1)      imgFormat = gl::Format::R8_UNORM;
		else if (nrChannels == 2) imgFormat = gl::Format::R8G8_UNORM;
		else if (nrChannels == 3) imgFormat = gl::Format::R8G8B8_UNORM;
		else if (nrChannels == 4) imgFormat = gl::Format::R8G8B8A8_UNORM;

		const gl::TextureCreateInfo createInfo{
		  .imageType   = gl::ImageType::Tex2D,
		  .format      = imgFormat,
		  .extent      = {static_cast<uint32_t>(imgW), static_cast<uint32_t>(imgH), 1},
		  .mipLevels   = 1,
		  .arrayLayers = 1,
		  .sampleCount = gl::SampleCount::Samples1,
		};
		m_textures[name] = new gl::Texture(createInfo, name);
		gl::Texture& texture = *m_textures[name];

		gl::UploadFormat texFormat{ gl::UploadFormat::RGB };
		if (nrChannels == 1)      texFormat = gl::UploadFormat::R;
		else if (nrChannels == 2) texFormat = gl::UploadFormat::RG;
		else if (nrChannels == 3) texFormat = gl::UploadFormat::RGB;
		else if (nrChannels == 4) texFormat = gl::UploadFormat::RGBA;

		texture.UpdateImage({
		  .extent = createInfo.extent,
		  .format = texFormat,
		  .type   = gl::UploadType::FLOAT,
		  .pixels = pixels,
			});
		stbi_image_free(pixels);

		Debug("Load Texture: " + name);

		return &texture;
	}
}
//=============================================================================