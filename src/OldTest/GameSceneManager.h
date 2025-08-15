#pragma once

#include "GameModelManager.h"
#include "ShadowPassManager.h"
#include "Light.h"

namespace sceneUBO
{
	struct alignas(16) SceneUniforms final
	{
		glm::vec3 CameraPos;
		int NumLight;
		glm::mat4 lightSpaceMatrix;
		glm::vec3 lightPos;
	};
}

class GameSceneManager final
{
public:
	bool Init();
	void Close();

	void Update();

	void SetModel(GameModelOld* model);

	void Draw(Camera& cam);
	void DrawInDepth(Camera& cam);

private:
	std::optional<gl::TypedBuffer<sceneUBO::SceneUniforms>> m_sceneUniformUbo;

	GameModelManager m_modelManager;

	// TODO: менеджер света
	std::vector<LightOld> m_lights;
	std::optional<gl::Buffer> m_lightSSBO;

	ShadowPassManager m_shadowPassMgr;
};