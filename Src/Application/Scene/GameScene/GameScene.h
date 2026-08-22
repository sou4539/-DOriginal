#pragma once

#include"../BaseScene/BaseScene.h"

class Status;

class GameScene : public BaseScene
{
public :

	// GameSceneを作成した時に、自動でInit()を呼んでゲーム用オブジェクトを配置する。
	GameScene()  { Init(); }

	// GameScene破棄時の処理。
	// m_objList内のshared_ptrが各オブジェクトを解放するため、ここでは追加処理を持たせていない。
	~GameScene() {}

private:

	// GameScene中の入力イベント処理。
	// 今はTキーでタイトルへ戻るデバッグ用処理を行う。
	void Event() override;

	// GameSceneの初期化処理。
	// カメラ、プレイヤー、敵、UI、ステージを作成して参照関係をつなぐ。
	void Init()  override;

	bool IsUpdatePaused() const override;
	bool CanUpdateWhenPaused(const std::shared_ptr<KdGameObject>& obj) const override;

	std::weak_ptr<Status> m_status;
};
