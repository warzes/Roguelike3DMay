#pragma once

#include "GameModelManager.h"

namespace sceneUBO
{
	struct SceneUniforms final
	{
		float NumLight;
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

	std::vector<Light>         m_lights;
	std::optional<gl4::Buffer> m_lightSSBO;
};