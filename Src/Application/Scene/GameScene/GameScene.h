#pragma once

#include"../BaseScene/BaseScene.h"
#include "../GameStageBase/StageTransition.h"


class GameStageBase;
class Status;
struct GrudgeBattleState;

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
	std::shared_ptr<GameStageBase> CreateGrassStage(GrassArrivalPoint arrival);
	// Statusをステージより長く保持し、HP・育成状態を引き継ぐ。
	std::shared_ptr<Status> m_status;
	// ステージ移動を越えるが、GameScene終了（タイトルへ戻る）時には破棄する。
	std::shared_ptr<GrudgeBattleState> m_grudgeBattle;
	std::shared_ptr<GameStageBase> m_stage;
	std::shared_ptr<GameStageBase> m_nextStage;
};
