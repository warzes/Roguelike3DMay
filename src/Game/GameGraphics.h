#pragma once

#include "GameModelManager.h"

class GameApp;
class GameSceneManager;

class GameGraphics final
{
public:
	bool Init(GameApp* gameApp);
	void Close();
	void Update(float deltaTime);
	void Render(GameSceneManager& scene);

	void Resize(uint16_t width, uint16_t height);

private:
	GameApp*                    m_gameApp{ nullptr };
	std::optional<gl4::Texture> m_colorBuffer;
	std::optional<gl4::Texture> m_depthBuffer;

	std::optional<gl4::Texture> m_depthBuffer2;
};