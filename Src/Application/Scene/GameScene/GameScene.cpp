#include "GameScene.h"
#include "../SceneManager.h"
#include "../GameStageBase/GameStageBase.h"
#include "../GameStageBase/GrassStage/GrassStage.h"
#include "../GameStageBase/CastleStage/CastleStage.h"
#include "../../GameObject/Character/Status/Status.h"
#include "../../GameObject/Character/Enemy/Bat/GrudgeBat.h"

GameScene::~GameScene() = default;

void GameScene::Init()
{
	m_status = std::make_shared<Status>();
	m_grudgeBattle = std::make_shared<GrudgeBattleState>();
	m_nextStage = CreateGrassStage(GrassArrivalPoint::VillageSpawn);
}

std::shared_ptr<GameStageBase> GameScene::CreateGrassStage(GrassArrivalPoint arrival)
{
	auto grass = std::make_shared<GrassStage>();
	grass->SetArrivalPoint(arrival);
	// 草原は入場を通知するだけ。移動先を決めるのはGameSceneの役割。
	// grassはこのGameSceneが所有するので、コールバックもその寿命内で使う。
	grass->SetCastleEntryCallback([this](CastleEntryRoute route)
	{
		auto castle = std::make_shared<CastleStage>();
		castle->SetEntryRoute(route);
		castle->SetGrudgeBattle(m_grudgeBattle);
		castle->SetExitCallback([this]() { SetNextStage(CreateGrassStage(GrassArrivalPoint::CastleEntrance)); });
		castle->SetDeathCallback([this]() { SetNextStage(CreateGrassStage(GrassArrivalPoint::VillageSpawn)); });
		castle->SetPortalReturnCallback([this]()
		{
			SetNextStage(CreateGrassStage(GrassArrivalPoint::VillagePortal));
		});
		SetNextStage(castle);
	});
	return grass;
}

void GameScene::SetNextStage(const std::shared_ptr<GameStageBase>& stage)
{
	if (!stage || stage == m_stage) { return; }
	m_nextStage = stage;
}

void GameScene::ChangeStage()
{
	if (!m_nextStage) { return; }
	// 死亡・ポータル・通常出口の全経路で、実際に退城するときに一度だけ減衰する。
	if (std::dynamic_pointer_cast<CastleStage>(m_stage) &&
		std::dynamic_pointer_cast<GrassStage>(m_nextStage) && m_grudgeBattle)
	{
		m_grudgeBattle->OnCastleExit();
	}
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
