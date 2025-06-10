#pragma once

namespace Utility
{
	struct Vertex
	{
		glm::vec3 position;
		uint32_t normal;
		glm::vec2 texcoord;
	};

	using index_t = uint32_t;

	struct Box3D
	{
		glm::vec3 offset;
		glm::vec3 halfExtent;
	};

	struct CombinedTextureSampler
	{
		gl4::TextureView texture;
		gl4::SamplerState sampler;
	};

	enum class MaterialFlagBit
	{
		HAS_BASE_COLOR_TEXTURE = 1 << 0,
	};
	SE_DECLARE_FLAG_TYPE(MaterialFlags, MaterialFlagBit, uint32_t)

	struct GpuMaterial
	{
		MaterialFlags flags{};
		float alphaCutoff{};
		uint32_t pad01{};
		uint32_t pad02{};
		glm::vec4 baseColorFactor{};
	};

	struct GpuMaterialBindless
	{
		MaterialFlags flags{};
		float alphaCutoff{};
		uint64_t baseColorTextureHandle{};
		glm::vec4 baseColorFactor{};
	};

	struct Material
	{
		GpuMaterial gpuMaterial{};
		std::optional<CombinedTextureSampler> albedoTextureSampler;
	};

	struct Mesh
	{
		//const GeometryBuffers* buffers;
		gl4::Buffer vertexBuffer;
		gl4::Buffer indexBuffer;
		uint32_t materialIdx{};
		glm::mat4 transform{};
	};

	struct Scene
	{
		std::vector<Mesh> meshes;
		std::vector<Material> materials;
	};

	struct MeshBindless
	{
		int32_t startVertex{};
		uint32_t startIndex{};
		uint32_t indexCount{};
		uint32_t materialIdx{};
		glm::mat4 transform{};
		Box3D boundingBox{};
	};

	struct SceneBindless
	{
		std::vector<MeshBindless> meshes;
		std::vector<Vertex> vertices;
		std::vector<index_t> indices;
		std::vector<GpuMaterialBindless> materials;
		std::vector<gl4::Texture> textures;
		std::vector<gl4::SamplerState> samplers;
	};

	bool LoadModelFromFile(Scene& scene,
		std::string_view fileName,
		glm::mat4 rootTransform = glm::mat4{ 1 },
		bool binary = false);

	bool LoadModelFromFileBindless(SceneBindless& scene,
		std::string_view fileName,
		glm::mat4 rootTransform = glm::mat4{ 1 },
		bool binary = false);
}