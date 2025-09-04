#include "stdafx.h"
#include "TextureManager.h"
#include "Log.h"
#include "FileUtils.h"
#include "OpenGL4ApiToEnum.h"
//=============================================================================
namespace
{
	std::unordered_map<std::string, gl::Texture*> texturesMap;
	gl::Texture*                                  defaultDiffuse2D{ nullptr };
	gl::Texture*                                  defaultNormal2D{ nullptr };
	gl::Texture*                                  defaultSpecular2D{ nullptr };
	bool                                          EnableSRGB{ false };
}
//=============================================================================
bool TextureManager::Init(bool sRGB)
{
	EnableSRGB = sRGB;

	// TODO: убрать копипаст

	// Create default diffuse texture
	{
		constexpr size_t SizeTexture = 32u;
		uint8_t data[SizeTexture][SizeTexture][3];
		for (size_t i = 0; i < SizeTexture; i++)
		{
			for (size_t j = 0; j < SizeTexture; j++)
			{
				if ((i + j) % 2 == 0)
				{
					data[i][j][0] = 255;
					data[i][j][1] = 0;
					data[i][j][2] = 100;
				}
				else
				{
					data[i][j][0] = 100;
					data[i][j][1] = 0;
					data[i][j][2] = 255;
				}
			}
		}

		const gl::TextureCreateInfo createInfo{
			.imageType   = gl::ImageType::Tex2D,
			.format      = gl::Format::R8G8B8_UNORM,
			.extent      = {SizeTexture, SizeTexture, 1u},
			.mipLevels   = 1u,
			.arrayLayers = 1u,
			.sampleCount = gl::SampleCount::Samples1,
		};
		defaultDiffuse2D = new gl::Texture(createInfo, "DefaultDiffuseTexture2D");
		defaultDiffuse2D->UpdateImage({
			.extent = createInfo.extent,
			.format = gl::UploadFormat::RGB,
			.type   = gl::UploadType::UBYTE,
			.pixels = data,
		});
	}

	// Create default normal texture
	{
		constexpr size_t SizeTexture = 8u;
		uint8_t data[SizeTexture][SizeTexture][3];
		for (size_t i = 0; i < SizeTexture; i++)
		{
			for (size_t j = 0; j < SizeTexture; j++)
			{
				data[i][j][0] = 128;
				data[i][j][1] = 128;
				data[i][j][2] = 255;
			}
		}

		const gl::TextureCreateInfo createInfo{
			.imageType = gl::ImageType::Tex2D,
			.format = gl::Format::R8G8B8_UNORM,
			.extent = {SizeTexture, SizeTexture, 1u},
			.mipLevels = 1u,
			.arrayLayers = 1u,
			.sampleCount = gl::SampleCount::Samples1,
		};
		defaultNormal2D = new gl::Texture(createInfo, "DefaultNormalTexture2D");
		defaultNormal2D->UpdateImage({
			.extent = createInfo.extent,
			.format = gl::UploadFormat::RGB,
			.type = gl::UploadType::UBYTE,
			.pixels = data,
			});
	}

	// Create default specular texture
	{
		constexpr size_t SizeTexture = 8u;
		uint8_t data[SizeTexture][SizeTexture][3];
		for (size_t i = 0; i < SizeTexture; i++)
		{
			for (size_t j = 0; j < SizeTexture; j++)
			{
				data[i][j][0] = 255;//Roughness
				data[i][j][1] = 255;//Metallic
				data[i][j][2] = 0;
			}
		}

		const gl::TextureCreateInfo createInfo{
			.imageType = gl::ImageType::Tex2D,
			.format = gl::Format::R8G8B8_UNORM,
			.extent = {SizeTexture, SizeTexture, 1u},
			.mipLevels = 1u,
			.arrayLayers = 1u,
			.sampleCount = gl::SampleCount::Samples1,
		};
		defaultSpecular2D = new gl::Texture(createInfo, "DefaultSpecularTexture2D");
		defaultSpecular2D->UpdateImage({
			.extent = createInfo.extent,
			.format = gl::UploadFormat::RGB,
			.type = gl::UploadType::UBYTE,
			.pixels = data,
			});
	}

	return true;
}
//=============================================================================
void TextureManager::Close()
{
	delete defaultDiffuse2D;
	defaultDiffuse2D = nullptr;
	delete defaultNormal2D;
	defaultNormal2D = nullptr;
	delete defaultSpecular2D;
	defaultSpecular2D = nullptr;

	for (auto& it : texturesMap)
	{
		delete it.second;
	}
	texturesMap.clear();
}
//=============================================================================
gl::Texture* TextureManager::GetDefaultDiffuse2D()
{
	return defaultDiffuse2D;
}
//=============================================================================
gl::Texture* TextureManager::GetDefaultNormal2D()
{
	return defaultNormal2D;
}
//=============================================================================
gl::Texture* TextureManager::GetDefaultSpecular2D()
{
	return defaultSpecular2D;
}
//=============================================================================
gl::Texture* TextureManager::GetTexture(const std::string& name, bool flipVertical)
{
	auto it = texturesMap.find(name);
	if (it != texturesMap.end())
	{
		return it->second;
	}
	else
	{
		bool hasTex = io::Exists(name);
		if (hasTex == false)
		{
			Error("Failed to load texture " + name);
			return GetDefaultDiffuse2D();
		}

		stbi_set_flip_vertically_on_load(flipVertical);

		int imgW, imgH, nrChannels;
		auto pixels = stbi_loadf(name.c_str(), &imgW, &imgH, &nrChannels, 0);
		if (!pixels || nrChannels < 1 || nrChannels > 4 || imgW < 0 || imgH < 0)
		{
			Error("Failed to load texture " + name);
			return GetDefaultDiffuse2D();
		}
		gl::Format imgFormat{ gl::Format::R8G8B8_UNORM };
		if (nrChannels == 1)      imgFormat = gl::Format::R8_UNORM;
		else if (nrChannels == 2) imgFormat = gl::Format::R8G8_UNORM;
		else if (nrChannels == 3) imgFormat = gl::Format::R8G8B8_UNORM;
		else if (nrChannels == 4) imgFormat = gl::Format::R8G8B8A8_UNORM;

		if (EnableSRGB) imgFormat = gl::detail::FormatToSrgb(imgFormat);

		const gl::TextureCreateInfo createInfo{
			.imageType   = gl::ImageType::Tex2D,
			.format      = imgFormat,
			.extent      = {static_cast<uint32_t>(imgW), static_cast<uint32_t>(imgH), 1},
			.mipLevels   = 1u,
			.arrayLayers = 1u,
			.sampleCount = gl::SampleCount::Samples1,
		};
		texturesMap[name] = new gl::Texture(createInfo, name);
		gl::Texture& texture = *texturesMap[name];

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