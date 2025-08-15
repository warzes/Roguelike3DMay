#pragma once

struct PhongMaterial final
{
	gl::Texture* diffuseTexture{ nullptr };
	gl::Texture* specularTexture{ nullptr };
	gl::Texture* normalTexture{ nullptr };
	gl::Texture* depthTexture{ nullptr };
	gl::Texture* emissionTexture{ nullptr };

	float heightScale{ 0.1f };
	float emissionStrength{ 1.0f };

	glm::vec3 diffuse{ 1.0f };
	glm::vec3 specular{ 1.0f };
	float     shininess{ 64.f };

	bool noLighing{ false };
};


// TODO: сделать возможность хранить буфер вершин/индексов в Model, а здесь хранить смещения в буфере
class Mesh final
{
public:
	Mesh() = default;
	Mesh(std::span<const MeshVertex> vertices,
		std::span<const uint32_t> indices,
		PhongMaterial* material);
	~Mesh();

	uint32_t GetVertexCount() const { return m_vertexCount; }
	uint32_t GetIndexCount() const { return m_indicesCount; }

	void Bind();

	PhongMaterial* GetMaterial() { return m_material; }

private:
	uint32_t       m_vertexCount{ 0 };
	uint32_t       m_indicesCount{ 0 };

	gl::Buffer* m_vertexBuffer{ nullptr };
	gl::Buffer* m_indexBuffer{ nullptr };
	PhongMaterial* m_material{ nullptr };
};

Mesh* LoadDataMesh(const std::vector<MeshVertex>& vertex, const std::vector<uint32_t>& indices, PhongMaterial* material = nullptr);
Mesh* LoadAssimpMesh(const std::string& filename);

struct GameModelOld final
{
	Mesh* mesh{ nullptr };

	PhongMaterial  material;
	gl::MagFilter textureFilter{};

	glm::vec3 position{ 0.0f };
	glm::vec3 rotation{ 0.0f }; // в градусах // TODO: заменить на кватернион?
	glm::vec3 scale{ 1.0f };

	glm::mat4 GetModelMat() const;
};

struct GameModel final
{
	void Free();

	std::vector<Mesh*> meshes;
	gl::MagFilter textureFilter{};

	AABB      aabb;
	glm::vec3 position{ 0.0f };
	glm::vec3 rotation{ 0.0f }; // в градусах // TODO: заменить на кватернион?
	glm::vec3 scale{ 1.0f };

	glm::mat4 GetModelMat() const;
};

std::optional<GameModel> LoadAssimpModel(const std::string& filename);