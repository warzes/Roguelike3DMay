﻿#pragma once

#include "OpenGL4Simple.h"

struct TextureFile final
{
	gl4::Texture2DId id;
	std::string name;
};

enum class TextureType
{
	NONE = 0,
	DIFFUSE = 1,
	NORMAL = 2,
	METALNESS = 3,
	ROUGHNESS = 4,
	AO = 5,
	EMISSIVE = 6,
};

// Mapping from ASSIMP textures to PBR textures
namespace TextureMapper
{
	// Corresponds to yhe number of elements in TextureType
	constexpr unsigned int NUM_TEXTURE_TYPE = 6;

	// This vector is a priority list
	static std::vector<aiTextureType> aiTTypeSearchOrder =
	{
		// Diffuse
		aiTextureType_DIFFUSE,

		// Metalness
		aiTextureType_SPECULAR,
		aiTextureType_METALNESS,

		// Normal
		aiTextureType_NORMALS,

		// Roughness
		aiTextureType_DIFFUSE_ROUGHNESS,
		aiTextureType_SHININESS,

		// AO
		aiTextureType_AMBIENT_OCCLUSION,
		aiTextureType_LIGHTMAP,

		// Emissive
		aiTextureType_EMISSIVE
	};

	static std::unordered_map<aiTextureType, TextureType> assimpTextureToTextureType =
	{
		// Diffuse
		{aiTextureType_DIFFUSE, TextureType::DIFFUSE},

		// Specular
		{aiTextureType_SPECULAR, TextureType::METALNESS},
		{aiTextureType_METALNESS, TextureType::METALNESS},

		// Normal
		{aiTextureType_NORMALS, TextureType::NORMAL},

		// Roughness shininess
		{aiTextureType_DIFFUSE_ROUGHNESS, TextureType::ROUGHNESS},
		{aiTextureType_SHININESS, TextureType::ROUGHNESS},

		// AO
		{aiTextureType_AMBIENT_OCCLUSION, TextureType::AO},
		{aiTextureType_LIGHTMAP, TextureType::AO},

		// Emissive
		{aiTextureType_EMISSIVE, TextureType::EMISSIVE}
	};

	static std::unordered_map<TextureType, std::string> textureTypeToString =
	{
		{TextureType::DIFFUSE,   "texture_diffuse"},
		{TextureType::NORMAL,    "texture_normal"},
		{TextureType::METALNESS, "texture_metalness"},
		{TextureType::ROUGHNESS, "texture_roughness"},
		{TextureType::AO,        "texture_ao"},
		{TextureType::EMISSIVE,  "texture_emissive"},
	};

	inline TextureType GetTextureType(aiTextureType aiTType)
	{
		if (!TextureMapper::assimpTextureToTextureType.contains(aiTType))
			return TextureType::NONE;

		return TextureMapper::assimpTextureToTextureType[aiTType];
	}

	inline std::string GetTextureString(TextureType tType)
	{
		if (!TextureMapper::textureTypeToString.contains(tType)) return "";
		return TextureMapper::textureTypeToString[tType];
	}
}