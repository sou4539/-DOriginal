#pragma once

#include "../BaseScene/BaseScene.h"

class Status;
class Player;
class TPSCamera;

// 草原・城に共通するステージ処理。
class GameStageBase : public BaseScene
{
public:
	GameStageBase() = default;
	~GameStageBase() override;

	// GameSceneから一度だけ開始する。育成状態は外から引き継ぐ。
	void Start(const std::shared_ptr<Status>& status);

protected:
	// 共通のイベント処理。
	void Event() override;

	// 共通の初期化。派生クラスのInitから呼ぶ。
	void Init()  override;
	void SetCursorVisible(bool isVisible);

	bool IsUpdatePaused() const override;
	bool CanUpdateWhenPaused(const std::shared_ptr<KdGameObject>& obj) const override;

	std::weak_ptr<Status> m_status;
	// 派生ステージで開始位置や地形との参照を設定する。
	std::weak_ptr<Player> m_player;
	std::weak_ptr<TPSCamera> m_camera;
	bool m_prevCursorDown = false;
	bool m_isCursorVisible = true;
	bool m_prevTitleDown = false;

private:
	bool m_started = false;
};
