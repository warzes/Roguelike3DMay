#pragma once

#include "UniformObjects.h"
#include "MainRenderPass.h"

class World;
class Camera;

class WorldRender final
{
public:
	WorldRender(World& world);

	bool Init();
	void Close();

	void Draw(Camera& cam);

private:
	World& m_world;

	MainRenderPass m_mainRenderPass;
};