#pragma once

#include "GameModelManager.h"

namespace sceneUBO
{
	struct SceneUniforms final
	{
		int NumDirectionalLight;
		int NumPointLight;
		glm::vec3 CameraPos;
	};
}

class GameSceneManager final
{
public:
	bool Init();
	void Close();

	void Update(Camera& cam);

	void SetModel(GameModel* model);

	void Draw();

private:
	GameModelManager m_modelManager;

	std::optional<gl4::TypedBuffer<sceneUBO::SceneUniforms>> m_sceneUniformUbo;

	std::vector<DirectionalLight> m_directionalLights;
	std::vector<PointLight> m_pointLights;
	std::optional<gl4::Buffer> m_directionalLightSSBO;
	std::optional<gl4::Buffer> m_pointLightSSBO;
};