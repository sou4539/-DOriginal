#include "GrassStage.h"


#include "../../../GameObject/Stage/Ground/Ground.h"
#include "../../../GameObject/Camera/TPSCamera/TPSCamera.h"
#include "../../../GameObject/Character/Player/Player.h"
#include "../../../GameObject/Character/Status/Status.h"
#include "../../../GameObject/Character/Enemy/EnemySpawner/EnemySpawner.h"
#include "../../../GameObject/Stage/Village/Village.h"
#include "../../../GameObject/Character/Castle/NoneCastle.h"
#include "../../../GameObject/Stage/Tree/Tree.h"


namespace
{
	constexpr float CastleWarpDistance = 40.0f;

}

void GrassStage::Init()
{
	GameStageBase::Init();
	auto player = m_player.lock();
	auto status = m_status.lock();
	auto camera = m_camera.lock();
	if (!player || !status || !camera) { return; }

	// 草原の開始位置と復活位置。
	const Math::Vector3 spawnPos = { 0.0f, 0.0f, -15.0f };
	player->SetPos(spawnPos);
	player->SetRespawnPos(spawnPos);

	auto ground = std::make_shared<Ground>();
	auto village = std::make_shared<Village>();
	auto tree = std::make_shared<Tree>();
	m_objList.push_back(ground);
	m_objList.push_back(village);
	m_objList.push_back(tree);
	// ランダム配置された城を保存し、デバッグワープで実際の位置を参照する。
	auto castle = std::make_shared<NoneCastle>(village->GetSafeAreaCenter());
	m_objList.push_back(castle);
	m_castle = castle;
	if (m_returnFromCastle)
	{
		player->SetPos(castle->GetPos() + Math::Vector3(0.0f, 0.0f, -CastleWarpDistance));
		player->SetAngle(DirectX::XM_PI);
		camera->SetYawDeg(180.0f);
	}

	m_prevWarpDown = (GetAsyncKeyState('B') & 0x8000) != 0;
	m_prevEnterDown = (GetAsyncKeyState('F') & 0x8000) != 0;

	// 草原用の敵の出現設定。
	auto enemySpawner = std::make_shared<EnemySpawner>();
	m_objList.push_back(enemySpawner);
	// グループごとの補充上限も合計200匹にする（30×2 + 28×5）。
	enemySpawner->AddSpawnArea({ -70.0f, 3.0f, 0.0f }, 16.0f, 30);
	enemySpawner->AddSpawnArea({ -30.0f, 3.0f, 55.0f }, 14.0f, 28);
	enemySpawner->AddSpawnArea({ -30.0f, 3.0f, -75.0f }, 14.0f, 28);
	enemySpawner->AddSpawnArea({ 55.0f, 3.0f, -45.0f }, 14.0f, 28);
	enemySpawner->AddSpawnArea({ -85.0f, 3.0f, 45.0f }, 14.0f, 28);
	enemySpawner->AddSpawnArea({ -85.0f, 3.0f, -45.0f }, 14.0f, 28);
	enemySpawner->AddSpawnArea({ -120.0f, 3.0f, 0.0f }, 16.0f, 30);
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

	auto player = m_player.lock();
	auto castle = m_castle.lock();
	auto camera = m_camera.lock();
	if (!player || !castle || !camera) { return; }

	// 仮の入口を城の-Z側40ユニットに置く。Bのワープ先も同じ位置にする。
	const Math::Vector3 warpPos = castle->GetPos() + Math::Vector3(0.0f, 0.0f, -CastleWarpDistance);
	if (warpPressed)
	{
		player->SetPos(warpPos);
		player->SetAngle(0.0f);
		// カメラも城へ向ける。復活位置は村のまま保つ。
		camera->SetYawDeg(0.0f);
		camera->ResetMouseMove();
	}

	if (!enterPressed || !m_onCastleEntry) { return; }
	// 入口から水平距離6ユニット以内でEを押したら、親へ入場を通知する。
	constexpr float EntranceRadius = 6.0f;
	Math::Vector3 toEntrance = warpPos - player->GetPos();
	toEntrance.y = 0.0f;
	if (toEntrance.LengthSquared() <= EntranceRadius * EntranceRadius)
	{
		// ここではステージを破棄しない。GameSceneが次フレームに切り替える。
		m_onCastleEntry();
	}
}
