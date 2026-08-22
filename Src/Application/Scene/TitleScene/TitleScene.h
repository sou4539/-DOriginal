#pragma once

#include"../BaseScene/BaseScene.h"

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

	bool m_prevLeftClick = false;
};


