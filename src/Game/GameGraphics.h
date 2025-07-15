#pragma once

#include "GameModelManager.h"

class GameApp;

class GameGraphics final
{
public:
	bool Init(GameApp* gameApp);
	void Close();
	void Update(float deltaTime);
	void Render();

	void Resize(uint16_t width, uint16_t height);

	void SetModel(GameModel* model);

private:
	GameApp*         m_gameApp{ nullptr };
	GameModelManager m_modelManager;

	std::optional<gl4::Texture> m_colorBuffer;
	std::optional<gl4::Texture> m_depthBuffer;
};