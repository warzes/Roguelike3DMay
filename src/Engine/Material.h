#pragma once

namespace gl
{
	class Texture;
}

struct PhongMaterial final
{
	gl::Texture* diffuseTexture{ nullptr };
	gl::Texture* specularTexture{ nullptr };
	gl::Texture* normalTexture{ nullptr };
	gl::Texture* depthTexture{ nullptr };
	gl::Texture* emissionTexture{ nullptr };

	float heightScale{ 0.1f };
	float emissionStrength{ 1.0f };

	glm::vec3 ambientColor{ 1.0f };
	glm::vec3 diffuseColor{ 1.0f };
	glm::vec3 specularColor{ 1.0f };
	float     shininess{ 64.f };
	float     refracti{ 0.0f };

	bool noLighing{ false };
};