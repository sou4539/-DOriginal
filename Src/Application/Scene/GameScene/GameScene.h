#pragma once

#include"../BaseScene/BaseScene.h"

class Status;

class GameScene : public BaseScene
{
public :

	// GameSceneを作成した時に、自動でInit()を呼んでゲーム用オブジェクトを配置する。
	GameScene()  { Init(); }

	// GameScene破棄時の処理。
	~GameScene();

private:

	// GameScene中の入力イベント処理。
	void Event() override;

	// GameSceneの初期化処理。
	void Init()  override;
	void SetCursorVisible(bool isVisible);

	bool IsUpdatePaused() const override;
	bool CanUpdateWhenPaused(const std::shared_ptr<KdGameObject>& obj) const override;

	std::weak_ptr<Status> m_status;
	bool m_isCursorVisible = true;
	bool m_prevBackTitleKey = false;
};
