#pragma once

#include "GameModel.h"
#include "Shadow.h"
#include "Light.h"

class World final
{
	friend class WorldRender;
public:
	bool Init();
	void Close();

	const std::vector<Light>& GetLights() const { return m_lights; }
	const std::vector<ShadowMap>& GetShadowMap() const { return m_shadows; }

private:
	GameModelOld m_model1;
	GameModelOld m_model2;
	GameModelOld m_model3;
	GameModelOld m_model4;

	std::vector<Light>     m_lights;
	std::vector<ShadowMap> m_shadows;
};