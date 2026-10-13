#pragma once

#include "../BaseScene/BaseScene.h"
#include "../../GameObject/UI/InteractionPrompt.h"

class Status;
class Player;
class TPSCamera;
class KdSoundInstance;
class PauseMenu;

// 草原・城に共通するステージ処理。
class GameStageBase : public BaseScene
{
public:
	GameStageBase() = default;
	~GameStageBase() override;

	// GameSceneから一度だけ開始する。育成状態は外から引き継ぐ。
	void Start(const std::shared_ptr<Status>& status);
	void DrawSprite() override;

protected:
	// 共通のイベント処理。
	void Event() override;

	// 共通の初期化。派生クラスのInitから呼ぶ。
	void Init()  override;
	void SetCursorVisible(bool isVisible);
	void StartStageBgm(const char* path);
	void SetMenuOpen(bool open);
	// 描画案内とFキーの操作が同じ条件を参照し、表示と判定のずれを防ぐ。
	virtual InteractionAction GetInteractionAction() const { return InteractionAction::None; }

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
	std::shared_ptr<KdSoundInstance> m_stageBgm;
	std::shared_ptr<InteractionPrompt> m_interactionPrompt;
	std::shared_ptr<PauseMenu> m_pauseMenu;
	// メニュー中はStatusの更新も止める。魔法選択中はStatusだけ更新を許可する。
	bool m_menuOpen = false;
	bool m_prevEscapeDown = false;
	bool m_cursorVisibleBeforeMenu = false;
	bool m_started = false;
};
