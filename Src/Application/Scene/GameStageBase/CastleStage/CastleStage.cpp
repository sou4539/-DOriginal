#include "CastleStage.h"

#include "../../../GameObject/Stage/Castle/Castle.h"
#include "../../../GameObject/Stage/WarpStone.h"
#include "../../../GameObject/Stage/Castle/CastleSafeArea.h"
#include "../../../GameObject/Stage/Ground/Ground.h"
#include "../../../GameObject/Camera/TPSCamera/TPSCamera.h"
#include "../../../GameObject/Character/Player/Player.h"
#include "../../../GameObject/Character/Status/Status.h"
#include "../../../GameObject/Character/Enemy/Bat/GrudgeBat.h"
#include "../../../GameObject/Character/Staff/StaffBase.h"

#include "../../SceneManager.h"

namespace
{
	const Math::Vector3 CastlePortalPos = { -30.0f, 0.0f, -5.0f };
}

// 城の入口付近にある封鎖された出口。開放状態を描画と退出判定で共有する。
class CastleExit final : public KdGameObject
{
public:
	CastleExit()
	{
		m_mWorld = Math::Matrix::CreateTranslation(-30.0f, 0.0f, -65.0f);
		m_wall = std::make_shared<KdSquarePolygon>();
		m_wall->SetScale(Math::Vector2(15.0f, 15.0f));
		m_wall->Set2DObject(false);
		m_pCollider = std::make_unique<KdCollider>();
		m_pCollider->RegisterCollisionShape("ExitWall", m_wall, KdCollider::TypeGround);
		m_gate = std::make_shared<KdSquarePolygon>();
		m_gate->SetMaterial("Asset/Textures/Gate/CastleGate.png");
		m_gate->SetScale(Math::Vector2(6.0f, 8.0f));
		m_gate->Set2DObject(false);
	}

	void SetOpen(bool open)
	{
		m_open = open;
	}

	bool IsTouching(const Math::Vector3& position) const
	{
		return m_open
			&& std::abs(position.x + 30.0f) <= 3.5f
			&& std::abs(position.z + 65.0f) <= 1.0f
			&& position.y >= -0.5f && position.y <= 7.5f;
	}

	void DrawUnLit() override
	{
		auto& shader = KdShaderManager::Instance().m_StandardShader;

		// 黒い壁は常に描画する。
		shader.DrawPolygon(
			*m_wall, m_mWorld, Math::Color(0, 0, 0, 1));

		// 敵が全滅したら出口画像を重ねる。
		if (m_open)
		{
			const Math::Matrix gateWorld =
				Math::Matrix::CreateTranslation(-30.0f, 4.0f, -64.95f);

			shader.DrawPolygon(*m_gate, gateWorld);
		}
	}



private:
	std::shared_ptr<KdSquarePolygon> m_wall;
	std::shared_ptr<KdSquarePolygon> m_gate;
	bool m_open = false;
};

void CastleStage::Init()
{
	GameStageBase::Init();

	// 城では杖を光らせる
	for (const auto& obj : m_objList)
	{
		if (auto staff = std::dynamic_pointer_cast<StaffBase>(obj))
		{
			staff->SetLightEnable(true);
		}
	}

	auto player = m_player.lock();
	auto status = m_status.lock();
	auto camera = m_camera.lock();
	if (!player || !status || !camera) { return; }
	StartStageBgm("Asset/Sounds/BGM/Game/CastleStage/CastleBGM.wav");
	player->SetDeathCallback(m_onDeath);

	// 城の開始位置と復活位置。
	const Math::Vector3 castleEntrancePos = { -30.0f, 0.0f, -50.0f };
	const Math::Vector3 spawnPos = m_entryRoute == CastleEntryRoute::VillagePortal ? CastlePortalPos : castleEntrancePos;
	player->SetPos(spawnPos);
	player->SetRespawnPos(spawnPos);

	// 草原と同じGroundを使い、地面の表示と当たり判定を登録する。
	auto ground = std::make_shared<Ground>();
	ground->SetTarget(player);
	// 画像追加後、次の行を有効にして実際のファイル名を指定する。
	ground->SetGroundTextures("Asset/Models/Objects/Stage/Ground/floor.png",
		"Asset/Models/Objects/Stage/Ground/floor_NormalMap.png");
	m_objList.push_back(ground);
	player->RegistHitObject(ground);

	// 出てくるコウモリの数を取得
	const int batCount = status->GetKillBatCount();

	// コウモリの数を初期化
	status->ResetKillBatCount();

	// キャスルステージの初期化処理
	std::shared_ptr<Castle> castle = std::make_shared<Castle>();
	castle->SetTarget(player);
	m_objList.push_back(castle);

	player->RegistHitObject(castle);
	m_objList.push_back(std::make_shared<WarpStone>(CastlePortalPos));

	// 中央の部屋の箱形安全地帯。復活位置は変更せず、独立して編集する。
	m_objList.push_back(std::make_shared<CastleSafeArea>(player));

	if (!m_grudgeBattle) { m_grudgeBattle = std::make_shared<GrudgeBattleState>(); }

	// 持ち越した残数に新たな草原の撃破数を追加する。復元分を合計へ二重加算しない。
	m_grudgeBattle->TotalCount += batCount;
	RefillGrudges();

	m_exit = std::make_shared<CastleExit>();
	m_exit->SetOpen(m_grudgeBattle->DefeatedCount >= m_grudgeBattle->TotalCount);
	m_objList.push_back(m_exit);
	player->RegistHitObject(m_exit);
	m_prevExitDown = (GetAsyncKeyState('F') & 0x8000) != 0;
	m_prevDebugSpawnDown = (GetAsyncKeyState('O') & 0x8000) != 0;

	camera->PostUpdate();
}

void CastleStage::CreateGrudges(int count, bool addToTotal)
{
	auto player = m_player.lock();
	auto status = m_status.lock();
	if (!player || !status) { return; }
	if (!m_grudgeBattle) { m_grudgeBattle = std::make_shared<GrudgeBattleState>(); }
	count = std::max(count, 0);
	const int firstIndex = m_grudgeBattle->TotalCount;
	if (addToTotal) { m_grudgeBattle->TotalCount += count; }
	// 中央の安全部屋を囲む16か所に分散する。
	const Math::Vector3 spawnPoints[] = {
		{-70, 2, -40}, {-50, 2, -40},
		{-30, 2, -40}, {-10, 2, -40},
		{ 10, 2, -40}, { 10, 2, -20},
		{ 10, 2,   0}, { 10, 2,  20},
		{ 10, 2,  40}, {-10, 2,  40},
		{-30, 2,  40}, {-50, 2,  40},
		{-70, 2,  40}, {-70, 2,  20},
		{-70, 2,   0}, {-70, 2, -20}

	};
	for (int i = 0; i < count; ++i)
	{
		auto grudge = std::make_shared<GrudgeBat>(m_grudgeBattle);
		grudge->SetTarget(player);
		grudge->SetStatus(status);
		Math::Vector3 pos = spawnPoints[(firstIndex + i) % 16];
		pos.x += KdRandom::GetFloat(-5.0f, 5.0f);
		pos.z += KdRandom::GetFloat(-5.0f, 5.0f);
		grudge->SetStartPos(pos);
		m_objList.push_back(grudge);
		m_grudges.push_back(grudge);
	}
	UpdateActiveGrudges();
}

void CastleStage::UpdateActiveGrudges()
{
	// 魔法の狙い・雷の連鎖などが使う敵リストにも怨念を登録する。
	std::vector<std::weak_ptr<EnemyBase>> activeEnemies;
	for (const auto& weakGrudge : m_grudges)
	{
		if (auto grudge = weakGrudge.lock(); grudge && grudge->CanBeTargeted())
		{
			activeEnemies.push_back(grudge);
		}
	}
	SceneManager::Instance().SetActiveEnemies(activeEnemies);
}

void CastleStage::RefillGrudges()
{
	if (!m_grudgeBattle) { return; }

	int aliveCount = 0;

	auto it = m_grudges.begin();
	while (it != m_grudges.end())
	{
		auto grudge = it->lock();

		if (!grudge)
		{
			it = m_grudges.erase(it);
			continue;
		}

		if (grudge->CanBeTargeted())
		{
			++aliveCount;
		}

		++it;
	}

	const int remaining = m_grudgeBattle->GetRemainingCount();

	const int emptySlots = std::max(
		0, GameBalance::CastleMaxGrudges - aliveCount);

	const int waitingCount = std::max(
		0, remaining - aliveCount);

	const int spawnCount = std::min(emptySlots, waitingCount);

	if (spawnCount > 0)
	{
		CreateGrudges(spawnCount, false);
	}
}

void CastleStage::Event()
{
	GameStageBase::Event();
	const bool exitDown = (GetAsyncKeyState('F') & 0x8000) != 0;
	const bool exitPressed = exitDown && !m_prevExitDown;
	m_prevExitDown = exitDown;
#ifdef _DEBUG
	const bool spawnDown = (GetAsyncKeyState('O') & 0x8000) != 0;
	const bool spawnPressed = spawnDown && !m_prevDebugSpawnDown;
	m_prevDebugSpawnDown = spawnDown;
#endif
	if (IsUpdatePaused()) { return; }
	if (exitPressed && GetInteractionAction() == InteractionAction::ReturnToVillage)
	{
		m_onPortalReturn();
		return;
	}
#ifdef _DEBUG
	if (spawnPressed)
	{
		m_grudgeBattle->TotalCount += 10;
	}
#endif
	if (m_exit && m_grudgeBattle)
	{
		m_exit->SetOpen(m_grudgeBattle->DefeatedCount >= m_grudgeBattle->TotalCount);
		auto player = m_player.lock();
		if (exitPressed && GetInteractionAction() == InteractionAction::ReturnToGrass)
		{
			m_onExit();
		}
	}

	Bat::UpdateSharedAnimation();
	RefillGrudges();
	UpdateActiveGrudges();
}

InteractionAction CastleStage::GetInteractionAction() const
{
	if (IsUpdatePaused()) { return InteractionAction::None; }
	auto player = m_player.lock();
	if (!player) { return InteractionAction::None; }
	const auto pos = player->GetPos();
	Math::Vector3 offset = pos - CastlePortalPos;
	offset.y = 0.0f;
	if (m_onPortalReturn && offset.LengthSquared() <= 6.0f * 6.0f
		&& pos.y >= CastlePortalPos.y - 0.5f && pos.y <= CastlePortalPos.y + 8.0f)
	{
		return InteractionAction::ReturnToVillage;
	}
	if (m_onExit && m_exit && m_exit->IsTouching(pos))
	{
		return InteractionAction::ReturnToGrass;
	}
	return InteractionAction::None;
}
