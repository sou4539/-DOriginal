#include "GrassStage.h"


#include "../../../GameObject/Stage/Ground/Ground.h"
#include "../../../GameObject/Camera/TPSCamera/TPSCamera.h"
#include "../../../GameObject/Character/Player/Player.h"
#include "../../../GameObject/Character/Status/Status.h"
#include "../../../GameObject/Character/Enemy/EnemySpawner/EnemySpawner.h"
#include "../../../GameObject/Stage/Village/Village.h"
#include "../../../GameObject/Character/Castle/NoneCastle.h"
#include "../../../GameObject/Stage/Tree/Tree.h"


void GrassStage::Init()
{
	GameStageBase::Init();
	auto player = m_player.lock();
	auto status = m_status.lock();
	auto camera = m_camera.lock();
	if (!player || !status || !camera) { return; }

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
	m_objList.push_back(std::make_shared<NoneCastle>(village->GetSafeAreaCenter()));

	// 草原用の敵の出現設定。
	auto enemySpawner = std::make_shared<EnemySpawner>();
	m_objList.push_back(enemySpawner);
	enemySpawner->AddSpawnArea({ -70.0f, 3.0f, 0.0f }, 16.0f, 15);
	enemySpawner->AddSpawnArea({ -30.0f, 3.0f, 55.0f }, 14.0f, 14);
	enemySpawner->AddSpawnArea({ -30.0f, 3.0f, -75.0f }, 14.0f, 14);
	enemySpawner->AddSpawnArea({ 55.0f, 3.0f, -45.0f }, 14.0f, 14);
	enemySpawner->AddSpawnArea({ -85.0f, 3.0f, 45.0f }, 14.0f, 14);
	enemySpawner->AddSpawnArea({ -85.0f, 3.0f, -45.0f }, 14.0f, 14);
	enemySpawner->AddSpawnArea({ -120.0f, 3.0f, 0.0f }, 16.0f, 15);
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

	// 開始位置の反映後にカメラを合わせる。
	camera->PostUpdate();
}
