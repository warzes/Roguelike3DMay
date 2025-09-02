#include "stdafx.h"
#include "MaterialManager.h"
//=============================================================================
bool MaterialManager::Init()
{
	m_nextMatID = 1;
	m_defaultMaterial.SetDiffuse("");
	m_defaultMaterial.SetNormal("");
	m_defaultMaterial.SetSpecular("");
	return true;
}
//=============================================================================
void MaterialManager::Close()
{
	RemoveAllMaterials();
}
//=============================================================================
Material* MaterialManager::CreateNewMaterial(unsigned int id, const std::string& name)
{
	if (m_materials.find(id) != m_materials.end())
		return m_materials[id];

	Material* newMat = new Material();
	newMat->setID(id);
	newMat->SetName(name);
	m_nextMatID = std::max(id + 1, m_nextMatID);
	m_materials[id] = newMat;

	return newMat;
}
//=============================================================================
Material* MaterialManager::CreateNewMaterial()
{
	return CreateNewMaterial("New Material " + std::to_string(m_nextMatID), false);
}
//=============================================================================
Material* MaterialManager::CreateNewMaterial(const std::string& name, bool findSame)
{
	if (findSame)
	{
		for (auto i = m_materials.begin(); i != m_materials.end(); i++)
		{
			if (i->second->GetName() == name)
				return i->second;
		}
	}

	Material* newMat = new Material();
	newMat->setID(m_nextMatID);
	m_nextMatID++;
	newMat->SetName(name);
	m_materials[newMat->GetID()] = newMat;

	return newMat;
}
//=============================================================================
Material* MaterialManager::GetMaterial(unsigned int id)
{
	if (m_materials.find(id) != m_materials.end())
		return m_materials[id];
	else
		return GetDefaultMaterial();
}
//=============================================================================
Material* MaterialManager::GetDefaultMaterial()
{
	return &m_defaultMaterial;
}
//=============================================================================
void MaterialManager::DeleteMaterial(unsigned int id)
{
	if (m_materials.find(id) != m_materials.end())
	{
		delete m_materials[id];
		m_materials.erase(id);
	}
}
//=============================================================================
void MaterialManager::RemoveAllMaterials()
{
	for (auto i = m_materials.begin(); i != m_materials.end(); i++)
	{
		delete i->second;
		m_materials.erase(i->first);
	}
}
//=============================================================================