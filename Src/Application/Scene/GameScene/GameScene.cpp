#include "GameScene.h"
#include "../SceneManager.h"
#include "../GameStageBase/GameStageBase.h"
#include "../GameStageBase/GrassStage/GrassStage.h"
#include "../../GameObject/Character/Status/Status.h"

GameScene::~GameScene() = default;

void GameScene::Init()
{
	m_status = std::make_shared<Status>();
	m_nextStage = std::make_shared<GrassStage>();
}

void GameScene::SetNextStage(const std::shared_ptr<GameStageBase>& stage)
{
	if (!stage || stage == m_stage) { return; }
	m_nextStage = stage;
}

void GameScene::ChangeStage()
{
	if (!m_nextStage) { return; }
	// 旧ステージを先に破棄し、その後で新しいカメラ・プレイヤーを作る。
	m_stage.reset();
	SceneManager::Instance().SetActiveEnemies({});
	m_stage = std::move(m_nextStage);
	m_status->SetEnemy({});
	m_status->SetVillageGuideRadius(0.0f);
	m_stage->Start(m_status);
}

void GameScene::PreUpdate()
{
	// オブジェクト更新中にステージを破棄しない。
	ChangeStage();
	if (m_stage) { m_stage->PreUpdate(); }
}

void GameScene::Update()
{
	if (m_stage) { m_stage->Update(); }
}

void GameScene::PostUpdate()
{
	if (m_stage) { m_stage->PostUpdate(); }
}

void GameScene::PreDraw()
{
	if (m_stage) { m_stage->PreDraw(); }
}

void GameScene::Draw()
{
	if (m_stage) { m_stage->Draw(); }
}

void GameScene::DrawSprite()
{
	if (m_stage) { m_stage->DrawSprite(); }
}

void GameScene::DrawDebug()
{
	if (m_stage) { m_stage->DrawDebug(); }
}

const std::list<std::shared_ptr<KdGameObject>>& GameScene::GetObjList()
{
	return m_stage ? m_stage->GetObjList() : m_objList;
}

void GameScene::AddObject(const std::shared_ptr<KdGameObject>& obj)
{
	// 魔法などの追加先も現在のステージへ転送する。
	if (m_stage) { m_stage->AddObject(obj); }
}
