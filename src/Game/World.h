#pragma once

#include "GameModel.h"

class World final
{
	friend class WorldRender;
public:
	bool Init();
	void Close();

private:
	GameModel m_model1;
	GameModel m_model2;
	GameModel m_model3;
};