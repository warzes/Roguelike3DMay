#pragma once

#include "Material.h"

class MaterialManager final
{
public:
	bool Init();
	void Close();

	Material* CreateNewMaterial(unsigned int id, const std::string& name);
	Material* CreateNewMaterial();
	Material* CreateNewMaterial(const std::string& name, bool findSame = true);
	Material* GetMaterial(unsigned int id);
	Material* GetDefaultMaterial();
	void DeleteMaterial(unsigned int id);
	void RemoveAllMaterials();
private:
	std::unordered_map<unsigned int, Material*> m_materials;
	Material                                    m_defaultMaterial;
	unsigned int                                m_nextMatID{ 0 };
};