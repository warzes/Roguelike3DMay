#pragma once

#include "Material.h"
#include "CoreFunc.h"

struct MeshVertex final
{
	glm::vec3 position{};
	glm::vec3 normal{};
	glm::vec2 uv{};
	glm::vec3 tangent{};

	bool operator==(const MeshVertex& v) const&
	{
		return position == v.position && normal == v.normal && uv == v.uv;
	}
};

namespace std
{
	template<> struct hash<MeshVertex>
	{
		std::size_t operator()(const MeshVertex& v) const noexcept
		{
			std::size_t h1 = std::hash<glm::vec3>{}((v.position));
			std::size_t h2 = std::hash<glm::vec3>{}((v.normal));
			std::size_t h3 = std::hash<glm::vec2>{}((v.uv));
			std::size_t seed = 0;
			HashCombine(seed, h1, h2, h3);
			return seed;
		}
	};
}

class Mesh final
{
public:
	Mesh() = default;
	Mesh(std::span<const MeshVertex> vertices,
		std::span<const uint32_t> indices,
		Material* material);

	uint32_t GetVertexCount() const { return m_vertexCount; }
	uint32_t GetIndexCount() const { return m_indicesCount; }

private:
	uint32_t m_vertexCount{ 0 };
	uint32_t m_indicesCount{ 0 };
};