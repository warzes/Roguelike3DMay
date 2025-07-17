#pragma once

#include "GameModelManager.h"
#include "ShadowPassManager.h"

namespace sceneUBO
{
	struct alignas(16) SceneUniforms final
	{
		glm::vec3 CameraPos;
		int NumLight;
	};
}

class GameSceneManager final
{
public:
	bool Init();
	void Close();

	void Update();

	void SetModel(GameModel* model);

	void Draw(Camera& cam);
	void DrawInDepth(Camera& cam);

private:
	std::optional<gl4::TypedBuffer<sceneUBO::SceneUniforms>> m_sceneUniformUbo;

	GameModelManager m_modelManager;

	// TODO: менеджер света
	std::vector<Light> m_lights;
	std::optional<gl4::Buffer> m_lightSSBO;

	ShadowPassManager m_shadowPassMgr;
};