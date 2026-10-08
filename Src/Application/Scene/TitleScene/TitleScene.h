#pragma once

#include "../BaseScene/BaseScene.h"

class KdSoundInstance;

class TitleScene : public BaseScene
{
public:

	TitleScene()  { Init(); }
	~TitleScene() override;

private:

	void Event() override;
	void Init()  override;
	void DrawSprite() override;

	// 毎フレームの入力処理。押した瞬間だけ操作を実行する。
	void UpdateProgressReset();
	void UpdateButtonInput();
	// 終了音を最後まで再生してからウィンドウを閉じる。
	void UpdateExitRequest();
	void StartGame();

	// 初期化は背景・音・UI画像の役割ごとに分ける。
	void CreateBackground();
	void StartTitleBgm();
	void LoadUITextures();

	// タイトルUIの画像。読み込みに失敗したものは描画を省略する。
	std::shared_ptr<KdTexture> m_startButtonTex = nullptr;
	std::shared_ptr<KdTexture> m_exitButtonTex = nullptr;
	std::shared_ptr<KdTexture> m_titleLogoTex = nullptr;
	std::shared_ptr<KdTexture> m_cursorTex = nullptr;
	// 終了音は再生終了を待つため、インスタンスを保持する。
	std::shared_ptr<KdSoundInstance> m_titleBgm = nullptr;
	std::shared_ptr<KdSoundInstance> m_exitClickSound = nullptr;

	// 前フレームの入力を記録し、押しっぱなしによる連続実行を防ぐ。
	bool m_prevLeftDown = false;
	bool m_prevResetDown = false;
	bool m_isExitRequested = false;
};


