#pragma once

// TODO: PBR Material

namespace gl4
{
	class Texture;
}

#define MATERIAL_DIFFUSE glm::vec3(1.0f)
#define MATERIAL_SPECULAR glm::vec3(1.0f)
#define MATERIAL_SHININESS 64.f

struct PhongMaterial final
{
	gl4::Texture* diffuseTexture{ nullptr };
	gl4::Texture* specularTexture{ nullptr };
	gl4::Texture* normalTexture{ nullptr };
	gl4::Texture* depthTexture{ nullptr };
	gl4::Texture* emissionTexture{ nullptr };

	float heightScale{ 0.1f };
	float emissionStrength{ 1.0f };

	glm::vec3 diffuse{ MATERIAL_DIFFUSE };
	glm::vec3 specular{ MATERIAL_SPECULAR };
	float     shininess{ MATERIAL_SHININESS };
};

class Material final
{
public:
	gl4::Texture* albedoTex{};
	gl4::Texture* roughnessTex{};
	gl4::Texture* metalnessTex{};
	gl4::Texture* normalTex{};
	gl4::Texture* ambientOcclusionTex{};

	// OLD ===>
	//int32_t baseColorTextureIndex{ -1 };
	//int32_t normalTextureIndex{ -1 };
	//int32_t emissiveTextureIndex{ -1 };
	//int32_t occlusionTextureIndex{ -1 };

	//int32_t metallicRoughnessTextureIndex{ -1 };
	//float metallicFactor{ 1.f };
	//float roughnessFactor{ 1.f };
	//uint32_t alphaMode{ 0 }; // 0: OPAQUE, 1: MASK

	//float alphaCutOff{ 0.5f };
	//uint32_t doubleSided{ 0 };

	//alignas(16) glm::vec4 baseColorFactor = glm::vec4(1.0f);
	//alignas(16) glm::vec3 emissiveFactor = glm::vec3(1.0f);
};

// sent to GPU
struct BindlessMaterial final
{
	uint64_t albedoHandle{};
	uint64_t roughnessHandle{};
	uint64_t metalnessHandle{};
	uint64_t normalHandle{};
	uint64_t ambientOcclusionHandle{};
};

class MaterialManager final
{
public:
	~MaterialManager();

	std::optional<Material> GetMaterial(const std::string& mat);
	Material& MakeMaterial(std::string name,
		std::string albedoTexName,
		std::string roughnessTexName,
		std::string metalnessTexName,
		std::string normalTexName,
		std::string ambientOcclusionTexName);

	std::vector<std::pair<std::string, Material>> GetLinearMaterials()
	{
		return { m_materials.begin(), m_materials.end() };
	}

private:
	std::unordered_map<std::string, Material> m_materials;
};