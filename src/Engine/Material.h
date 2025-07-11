#pragma once

// TODO: PBR Material

namespace gl4
{
	class Texture;
}

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