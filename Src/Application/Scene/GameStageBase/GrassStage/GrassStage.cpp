#include "GrassStage.h"


#include "../../../GameObject/Stage/Ground/Ground.h"
#include "../../../GameObject/Camera/TPSCamera/TPSCamera.h"
#include "../../../GameObject/Character/Player/Player.h"
#include "../../../GameObject/Character/Status/Status.h"
#include "../../../GameObject/Character/Enemy/EnemySpawner/EnemySpawner.h"
#include "../../../GameObject/Stage/Village/Village.h"
#include "../../../GameObject/Character/Castle/NoneCastle.h"
#include "../../../GameObject/Stage/Tree/Tree.h"
#include "../../../GameObject/Stage/WarpStone.h"

#include "../../../main.h"


namespace
{
	const Math::Vector3 VillagePortalPos = { 0.0f, 0.0f, 35.0f };
	constexpr float EntranceRadius = 6.0f;
	constexpr float VillageHealPerSecond = 10.0f;
}

void GrassStage::PostUpdate()
{
	GameStageBase::PostUpdate();

	if (IsUpdatePaused()) { return; }

	auto player = m_player.lock();
	auto status = m_status.lock();

	if (!player || !status) { return; }
	if (!player->IsInSafeArea()) { return; }

	status->HealPlayer(
		VillageHealPerSecond *
		Application::Instance().GetDeltaTime());
}

void GrassStage::Init()
{
	GameStageBase::Init();
	auto player = m_player.lock();
	auto status = m_status.lock();
	auto camera = m_camera.lock();
	if (!player || !status || !camera) { return; }
	StartStageBgm("Asset/Sounds/BGM/Game/GrassStage/GrassBGM.wav");

	// 草原の開始位置と復活位置。
	const Math::Vector3 spawnPos = { -30.0f, 0.0f, 0.0f };
	player->SetPos(spawnPos);
	player->SetRespawnPos(spawnPos);

	auto ground = std::make_shared<Ground>();
	auto village = std::make_shared<Village>();
	auto tree = std::make_shared<Tree>();
	m_objList.push_back(ground);
	m_objList.push_back(village);
	m_objList.push_back(tree);
	m_objList.push_back(std::make_shared<WarpStone>(VillagePortalPos));
	// ランダム配置された城を保存し、デバッグワープで実際の位置を参照する。
	auto castle = std::make_shared<NoneCastle>(village->GetSafeAreaCenter());
	m_objList.push_back(castle);
	m_castle = castle;
	if (m_arrivalPoint == GrassArrivalPoint::VillagePortal)
	{
		player->SetPos(VillagePortalPos);
		player->SetAngle(DirectX::XM_PI);
		camera->SetYawDeg(180.0f);
	}
	else if (m_arrivalPoint == GrassArrivalPoint::CastleEntrance)
	{
		player->SetPos(castle->GetEntrancePosition());
		player->SetAngle(DirectX::XM_PI);
		camera->SetYawDeg(180.0f);
	}

	m_prevWarpDown = (GetAsyncKeyState('B') & 0x8000) != 0;
	m_prevEnterDown = (GetAsyncKeyState('F') & 0x8000) != 0;

	// 草原用の敵の出現設定。
	auto enemySpawner = std::make_shared<EnemySpawner>();
	m_objList.push_back(enemySpawner);
	// 最大数を7グループへ分配し、上限設定を変えても合計を一致させる。
	const Math::Vector3 spawnCenters[] = {
		{-70, 3, 0}, {-30, 3, 55}, {-30, 3, -75}, {55, 3, -45},
		{-85, 3, 45}, {-85, 3, -45}, {-120, 3, 0}
	};
	constexpr int groupCount = 7;
	for (int i = 0; i < groupCount; ++i)
	{
		const int count = GameBalance::GrassMaxEnemies / groupCount
			+ (i < GameBalance::GrassMaxEnemies % groupCount ? 1 : 0);
		enemySpawner->AddSpawnArea(spawnCenters[i], 16.0f, count);
	}
	enemySpawner->SetStatus(status);

	// 村の安全地帯と案内範囲。
	village->SetVisibleRadius(160.0f);
	status->SetVillageGuideRadius(village->GetVisibleRadius());
	player->SetSafeArea(village->GetSafeAreaCenter(), village->GetSafeAreaRadius());
	enemySpawner->SetSafeArea(village->GetSafeAreaCenter(), village->GetSafeAreaRadius());
	enemySpawner->AddEnemiesToScene(m_objList, player);

	ground->SetTarget(player);
	village->SetTarget(player);
	tree->SetTarget(player);
	player->RegistHitObject(ground);
	player->RegistHitObject(village);
	player->RegistHitObject(tree);
	player->RegistHitObject(castle);

	// 開始位置の反映後にカメラを合わせる。
	camera->PostUpdate();
}

void GrassStage::Event()
{
	// F1のカーソル切替とTのタイトル移動も引き続き処理する。
	GameStageBase::Event();

	const bool warpDown = (GetAsyncKeyState('B') & 0x8000) != 0;
	const bool warpPressed = warpDown && !m_prevWarpDown;
	m_prevWarpDown = warpDown;
	// 押しっぱなしで入場予約を繰り返さないよう、Fも押した瞬間だけ調べる。
	const bool enterDown = (GetAsyncKeyState('F') & 0x8000) != 0;
	const bool enterPressed = enterDown && !m_prevEnterDown;
	m_prevEnterDown = enterDown;
	if (IsUpdatePaused()) { return; }

	auto player = m_player.lock();
	auto castle = m_castle.lock();
	auto camera = m_camera.lock();
	if (!player || !castle || !camera) { return; }

	// モデルの中央を基準にした入口。Bのワープ先も同じ位置にする。
	const Math::Vector3 warpPos = castle->GetEntrancePosition();
	if (warpPressed)
	{
		player->SetPos(warpPos);
		player->SetAngle(0.0f);
		// カメラも城へ向ける。復活位置は村のまま保つ。
		camera->SetYawDeg(0.0f);
		camera->ResetMouseMove();
	}

	if (enterPressed && GetInteractionAction() == InteractionAction::EnterCastle)
	{
		m_onCastleEntry(IsNearVillagePortal() ? CastleEntryRoute::VillagePortal : CastleEntryRoute::Entrance);
	}
}

bool GrassStage::IsNearVillagePortal() const
{
	auto player = m_player.lock();
	if (!player) { return false; }
	const auto pos = player->GetPos();
	Math::Vector3 offset = pos - VillagePortalPos;
	offset.y = 0.0f;
	return offset.LengthSquared() <= EntranceRadius * EntranceRadius
		&& pos.y >= VillagePortalPos.y - 0.5f && pos.y <= VillagePortalPos.y + 8.0f;
}

InteractionAction GrassStage::GetInteractionAction() const
{
	if (IsUpdatePaused() || !m_onCastleEntry) { return InteractionAction::None; }
	auto player = m_player.lock();
	auto castle = m_castle.lock();
	if (!player || !castle) { return InteractionAction::None; }
	return IsNearVillagePortal() || castle->CanEnter(player->GetPos())
		? InteractionAction::EnterCastle : InteractionAction::None;
}
