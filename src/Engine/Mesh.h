#pragma once

#include "Material.h"
#include "CoreFunc.h"
#include "OpenGL4Buffer.h"
#include "OpenGL4Pipeline.h"

struct MeshVertex final
{
	glm::vec3 position{};
	glm::vec3 color{};
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

constexpr std::array<gl4::VertexInputBindingDescription, 5> MeshVertexInputBindingDescs{
  gl4::VertexInputBindingDescription{
	.location = 0,
	.binding = 0,
	.format = gl4::Format::R32G32B32_FLOAT,
	.offset = offsetof(MeshVertex, position),
  },
	gl4::VertexInputBindingDescription{
	.location = 1,
	.binding = 0,
	.format = gl4::Format::R32G32B32_FLOAT,
	.offset = offsetof(MeshVertex, color),
  },
  gl4::VertexInputBindingDescription{
	.location = 2,
	.binding = 0,
	.format = gl4::Format::R32G32B32_FLOAT,
	.offset = offsetof(MeshVertex, normal),
  },
	gl4::VertexInputBindingDescription{
	.location = 3,
	.binding = 0,
	.format = gl4::Format::R32G32_FLOAT,
	.offset = offsetof(MeshVertex, uv),
  },
	gl4::VertexInputBindingDescription{
	.location = 4,
	.binding = 0,
	.format = gl4::Format::R32G32B32_FLOAT,
	.offset = offsetof(MeshVertex, tangent),
  },
};

// TODO: сделать возможность хранить буфер вершин/индексов в Model, а здесь хранить смещения в буфере
class Mesh final
{
public:
	Mesh() = default;
	Mesh(std::span<const MeshVertex> vertices,
		std::span<const uint32_t> indices,
		std::optional<Material> material);
	~Mesh();

	uint32_t GetVertexCount() const { return m_vertexCount; }
	uint32_t GetIndexCount() const { return m_indicesCount; }

	void Bind();

private:
	uint32_t m_vertexCount{ 0 };
	uint32_t m_indicesCount{ 0 };

	gl4::Buffer* m_vertexBuffer{ nullptr };
	gl4::Buffer* m_indexBuffer{ nullptr };
	std::optional<Material> m_material{};
};