#pragma once

#include"../BaseScene/BaseScene.h"


class GameStageBase;
class Status;

// 現在のステージを管理し、更新・描画を委譲する。
class GameScene : public BaseScene
{
public:
	GameScene() { Init(); }
	~GameScene() override;
	void PreUpdate() override;
	void Update() override;
	void PostUpdate() override;
	void PreDraw() override;
	void Draw() override;
	void DrawSprite() override;
	void DrawDebug() override;
	const std::list<std::shared_ptr<KdGameObject>>& GetObjList() override;
	void AddObject(const std::shared_ptr<KdGameObject>& obj) override;

	// 未開始の新しいステージを予約。次フレーム先頭で切り替える。
	void SetNextStage(const std::shared_ptr<GameStageBase>& stage);
	std::shared_ptr<GameStageBase> GetStage() const { return m_stage; }

private:
	void Init() override;
	void ChangeStage();
	// Statusをステージより長く保持し、HP・育成状態を引き継ぐ。
	std::shared_ptr<Status> m_status;
	std::shared_ptr<GameStageBase> m_stage;
	std::shared_ptr<GameStageBase> m_nextStage;
};
