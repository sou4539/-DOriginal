#include "CastleStage.h"

#include "../../../GameObject/Stage/Castle/Castle.h"
#include "../../../GameObject/Camera/TPSCamera/TPSCamera.h"
#include "../../../GameObject/Character/Player/Player.h"
#include "../../../GameObject/Character/Status/Status.h"

void CastleStage::Init()
{
	GameStageBase::Init();
	auto player = m_player.lock();
	auto status = m_status.lock();
	auto camera = m_camera.lock();
	if (!player || !status || !camera) { return; }

	// 城の開始位置と復活位置。
	const Math::Vector3 spawnPos = { -30.0f, 0.0f, 0.0f };
	player->SetPos(spawnPos);
	player->SetRespawnPos(spawnPos);

	// キャスルステージの初期化処理
	std::shared_ptr<Castle> castle = std::make_shared<Castle>();
	m_objList.push_back(castle);

	player->RegistHitObject(castle);
}
