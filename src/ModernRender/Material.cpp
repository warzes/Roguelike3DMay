#include "stdafx.h"
#include "Material.h"
//=============================================================================
void Material::SetName(const std::string& n)
{
	m_name = n;
}
//=============================================================================
void Material::SetRoughness(float r)
{
	m_roughness = glm::clamp(r, 0.0f, 1.0f);
}
//=============================================================================
void Material::SetMetallic(float m)
{
	m_metallic = glm::clamp(m, 0.0f, 1.0f);
}
//=============================================================================
void Material::SetDiffuse(const std::string& dir)
{
	setTexture(dir, 0);
}
//=============================================================================
void Material::SetNormal(const std::string& dir)
{
	setTexture(dir, 1);
}
//=============================================================================
void Material::SetSpecular(const std::string& dir)
{
	setTexture(dir, 2);
}
//=============================================================================
void Material::SetColor(const glm::vec4& c)
{
	m_color = c;
}
//=============================================================================
void Material::SetDiffuseTilling(float x, float y)
{
	m_texTilling[0] = x;
	m_texTilling[1] = y;
}
//=============================================================================
void Material::SetNormalTilling(float x, float y)
{
	m_texTilling[2] = x;
	m_texTilling[3] = y;
}
//=============================================================================
void Material::SetSpecularTilling(float x, float y)
{
	m_texTilling[4] = x;
	m_texTilling[5] = y;
}
//=============================================================================
void Material::SetEmissionColor(const glm::vec3& c)
{
	m_emissionColor = c;
}
//=============================================================================
void Material::SetEmissionIntensity(float i)
{
	m_emissionIntensity = i;
}
//=============================================================================
void Material::SetAlphaTestThreshold(float t)
{
	m_alphaTestThreshold = t;
}
//=============================================================================
void Material::RemoveDiffuse()
{
	setTexture("", 0);
}
//=============================================================================
void Material::RemoveNormal()
{
	setTexture("", 1);
}
//=============================================================================
void Material::RemoveSpecular()
{
	setTexture("", 2);
}
//=============================================================================
unsigned int Material::GetID() const
{
	return m_id;
}
//=============================================================================
std::string Material::GetName() const
{
	return m_name;
}
//=============================================================================
float Material::GetMetallic() const
{
	return m_metallic;
}
//=============================================================================
float Material::GetRoughness() const
{
	return m_roughness;
}
//=============================================================================
std::string Material::GetDiffusePath() const
{
	return m_texDirs[0];
}
//=============================================================================
gl::Texture* Material::GetDiffuse() const
{
	return m_texIns[0];
}
//=============================================================================
std::string Material::GetNormalPath() const
{
	return m_texDirs[1];
}
//=============================================================================
gl::Texture* Material::GetNormal() const
{
	return m_texIns[1];
}
//=============================================================================
std::string Material::GetSpecularPath() const
{
	return m_texDirs[2];
}
//=============================================================================
gl::Texture* Material::GetSpecular() const
{
	return m_texIns[2];
}
//=============================================================================
glm::vec4 Material::GetColor() const
{
	return m_color;
}
//=============================================================================
glm::vec3 Material::GetDiffuseTilling() const
{
	return glm::vec3(m_texTilling[0], m_texTilling[1], 0.0f);
}
//=============================================================================
glm::vec3 Material::GetNormalTilling() const
{
	return glm::vec3(m_texTilling[2], m_texTilling[3], 0.0f);
}
//=============================================================================
glm::vec3 Material::GetSpecularTilling() const
{
	return glm::vec3(m_texTilling[4], m_texTilling[5], 0.0f);
}
//=============================================================================
glm::vec3 Material::GetEmissionColor() const
{
	return m_emissionColor;
}
//=============================================================================
float Material::GetEmissionIntensity() const
{
	return m_emissionIntensity;
}
//=============================================================================
float Material::GetAlphaTestThreshold() const
{
	return m_alphaTestThreshold;
}
//=============================================================================
bool Material::HasDiffuseTexture() const
{
	return m_hasTex[0];
}
//=============================================================================
bool Material::HasNormalTexture() const
{
	return m_hasTex[1];
}
//=============================================================================
bool Material::HasSpecularTexture() const
{
	return m_hasTex[2];
}
//=============================================================================
Material::Material()
{
	m_id = 0;
	m_name = "New Material";
	m_roughness = 0.5f;
	m_metallic = 0.5f;
	m_texIns[0] = 0;
	m_texIns[1] = 0;
	m_texIns[2] = 0;
	m_hasTex[0] = false;
	m_hasTex[1] = false;
	m_hasTex[2] = false;
	m_texTilling[0] = 1.0f;
	m_texTilling[1] = 1.0f;
	m_texTilling[2] = 1.0f;
	m_texTilling[3] = 1.0f;
	m_texTilling[4] = 1.0f;
	m_texTilling[5] = 1.0f;
	m_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	m_emissionColor = glm::vec3(0.0f, 0.0f, 0.0f);
	m_emissionIntensity = 1.0f;
	m_alphaTestThreshold = 0.5f;
	SetDiffuse("");
	SetNormal("");
	SetSpecular("");
}
//=============================================================================
Material::~Material()
{
	RemoveDiffuse();
	RemoveNormal();
	RemoveSpecular();
}
//=============================================================================
void Material::setTexture(const std::string& dir, int index)
{
	m_texDirs[index] = dir;
	if (dir.empty())
	{
		if (index == 0) m_texIns[index] = TextureManager::GetDefaultDiffuse2D();
		else if(index == 1) m_texIns[index] = TextureManager::GetDefaultNormal2D();
		else if (index == 2) m_texIns[index] = TextureManager::GetDefaultSpecular2D();
		m_hasTex[index] = false;
	}
	else
	{
		m_texIns[index] = TextureManager::GetTexture(dir);
		if (m_texIns[index] == 0)
		{
			// TODO: копипаст
			if (index == 0) m_texIns[index] = TextureManager::GetDefaultDiffuse2D();
			else if (index == 1) m_texIns[index] = TextureManager::GetDefaultNormal2D();
			else if (index == 2) m_texIns[index] = TextureManager::GetDefaultSpecular2D();
			m_hasTex[index] = false;
		}
		else
			m_hasTex[index] = true;
	}
}
//=============================================================================
void Material::setID(unsigned int newID)
{
	m_id = newID;
}
//=============================================================================