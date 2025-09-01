#pragma once

#include "CoreFunc.h"
#include "OpenGL4Pipeline.h"

struct MeshVertex final
{
	glm::vec3 position{ 0.0f };
	glm::vec3 color{ 1.0f };
	glm::vec3 normal{ 0.0f };
	glm::vec2 texCoord{ 0.0f };
	glm::vec3 tangent{ 0.0f };

	bool operator==(const MeshVertex& v) const&
	{
		return position == v.position && normal == v.normal && texCoord == v.texCoord;
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
			std::size_t h3 = std::hash<glm::vec2>{}((v.texCoord));
			std::size_t seed = 0;
			HashCombine(seed, h1, h2, h3);
			return seed;
		}
	};
}

constexpr std::array<gl::VertexInputBindingDescription, 5> MeshVertexInputBindingDesc{
  gl::VertexInputBindingDescription{
	.location = 0,
	.binding = 0,
	.format = gl::Format::R32G32B32_FLOAT,
	.offset = offsetof(MeshVertex, position),
  },
	gl::VertexInputBindingDescription{
	.location = 1,
	.binding = 0,
	.format = gl::Format::R32G32B32_FLOAT,
	.offset = offsetof(MeshVertex, color),
  },
  gl::VertexInputBindingDescription{
	.location = 2,
	.binding = 0,
	.format = gl::Format::R32G32B32_FLOAT,
	.offset = offsetof(MeshVertex, normal),
  },
	gl::VertexInputBindingDescription{
	.location = 3,
	.binding = 0,
	.format = gl::Format::R32G32_FLOAT,
	.offset = offsetof(MeshVertex, texCoord),
  },
	gl::VertexInputBindingDescription{
	.location = 4,
	.binding = 0,
	.format = gl::Format::R32G32B32_FLOAT,
	.offset = offsetof(MeshVertex, tangent),
  },
};