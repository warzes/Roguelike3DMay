#pragma once

#include "UniformObjects.h"
#include "ShadowPass.h"
#include "MainRenderPass.h"


class World;
class Camera;

constexpr size_t MaxWorldModelDraw = 1'000'000;

class WorldRender final
{
public:
	WorldRender(World& world);

	bool Init();
	void Close();

	void BeginFrame();
	void StartShadowPass();
	void StartMainRenderPass(Camera& cam, const glm::mat4& proj);
	void EndFrame();

private:
	void setDrawModel(GameModel* model);

	World&                  m_world;
	std::vector<GameModel*> m_models{ MaxWorldModelDraw };
	size_t                  m_currentModel{ 0 };

	ShadowPass              m_shadowPass;
	MainRenderPass          m_mainRenderPass;
};