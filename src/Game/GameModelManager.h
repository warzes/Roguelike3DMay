#pragma once

constexpr size_t MaxModelDraw = 1'000'000;

/*
Класс отвечающий за работу с моделями/мешами в рамках игры
*/

namespace modelUBO
{
	struct GlobalUniforms final
	{
		glm::mat4 view;
		glm::mat4 proj;
	};

	struct ObjectUniforms final
	{
		glm::mat4 model;
		float NumLight;
	};
}

Mesh* LoadDataMesh(const std::vector<MeshVertex>& vertex, const std::vector<uint32_t>& indices);
Mesh* LoadAssimpMesh(const std::string& filename);

struct GameModel final
{
	Mesh*          mesh{ nullptr };

	gl4::Texture*  diffuse{ nullptr };
	gl4::MagFilter textureFilter{};

	glm::vec3 position{ 0.0f };
	glm::vec3 rotation{ 0.0f }; // в градусах // TODO: заменить на кватернион?
	glm::vec3 scale{ 1.0f };
	glm::mat4 GetModelMat() const;
};

class GameModelManager final
{
public:
	bool Init();
	void Close();

	void Update(Camera& cam);

	void SetModel(GameModel* model);

	void Draw();

private:
	bool createPipeline();
	std::optional<gl4::GraphicsPipeline>                      m_pipeline;
	std::optional<gl4::TypedBuffer<modelUBO::GlobalUniforms>> m_globalUniformsUbo;
	std::optional<gl4::TypedBuffer<modelUBO::ObjectUniforms>> m_objectUniformUbo;

	std::optional<gl4::Sampler>                               m_nearestSampler;
	std::optional<gl4::Sampler>                               m_linearSampler;

	std::vector<GameModel*> m_models;
	size_t                  m_currentModel{ 0 };

	std::vector<Light> m_lights;
	std::optional<gl4::Buffer> m_lightSSBO;
};