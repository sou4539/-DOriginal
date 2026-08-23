#pragma once

#include"../BaseScene/BaseScene.h"

class KdSoundInstance;

class TitleScene : public BaseScene
{
public :

	TitleScene()  { Init(); }
	~TitleScene() {}

private :

	void Event() override;
	void Init()  override;
	void DrawSprite() override;

	std::shared_ptr<KdTexture> m_startButtonTex = nullptr;
	std::shared_ptr<KdTexture> m_exitButtonTex = nullptr;
	std::shared_ptr<KdTexture> m_titleLogoTex = nullptr;
	std::shared_ptr<KdTexture> m_cursorTex = nullptr;
	std::shared_ptr<KdSoundInstance> m_exitClickSound = nullptr;

	bool m_prevLeftClick = false;
	bool m_prevDebugResetKey = false;
	bool m_isExitRequested = false;
};


