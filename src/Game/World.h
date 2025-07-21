#pragma once

#include "GameModel.h"

class World final
{
	friend class WorldRender;
public:
	bool Init();
	void Close();

	const std::vector<Light>& GetLights() const { return m_lights; }
	const std::vector<ShadowMap>& GetShadowMap() const { return m_shadows; }

private:
	GameModel m_model1;
	GameModel m_model2;
	GameModel m_model3;
	GameModel m_model4;

	std::vector<Light>     m_lights;
	std::vector<ShadowMap> m_shadows;
};