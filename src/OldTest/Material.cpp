#include "stdafx.h"
#include "Material.h"
//=============================================================================
MaterialManager::~MaterialManager()
{
	for (auto& p : m_materials)
	{
		delete p.second.albedoTex;
		delete p.second.roughnessTex;
		delete p.second.metalnessTex;
		delete p.second.normalTex;
		delete p.second.ambientOcclusionTex;
	}
}
//=============================================================================
std::optional<Material> MaterialManager::GetMaterial(const std::string& mat)
{
	auto it = m_materials.find(mat);
	if (it == m_materials.end())
	{
		return std::optional<Material>();
	}
	return it->second;
}
//=============================================================================
Material& MaterialManager::MakeMaterial(std::string name,
	std::string albedoTexName,
	std::string roughnessTexName,
	std::string metalnessTexName,
	std::string normalTexName,
	std::string ambientOcclusionTexName)
{
	if (auto it = m_materials.find(name); it != m_materials.end())
		return it->second;

	// Load Texture
	Material material;
	material.albedoTex = TextureManager::GetTexture(albedoTexName, gl::ColorSpace::sRGB);
	material.roughnessTex = TextureManager::GetTexture(roughnessTexName);
	material.metalnessTex = TextureManager::GetTexture(metalnessTexName);
	material.normalTex = TextureManager::GetTexture(normalTexName);
	material.ambientOcclusionTex = TextureManager::GetTexture(ambientOcclusionTexName);
	auto p = m_materials.insert({ name, material });
	return p.first->second;
}
//=============================================================================