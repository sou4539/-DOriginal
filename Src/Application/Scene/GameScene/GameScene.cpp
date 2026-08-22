#include "GameScene.h"
#include "../SceneManager.h"

#include "../../GameObject/Camera/TPSCamera/TPSCamera.h"
#include "../../GameObject/Character/Bat/BatGroup.h"
#include "../../GameObject/Character/Player/Player.h"
//#include "../../GameObject/Character/Enemy/Enemy.h"
#include "../../GameObject/Character/Status/Status.h"
#include "../../GameObject/Stage/Ground/Ground.h"
#include "../../GameObject/Stage/Village/Village.h"
#include "../../GameObject/Character/Staff/FireStaff/FireStaff.h"
#include "../../GameObject/Character/Staff/IceStaff/IceStaff.h"
#include "../../GameObject/Character/Staff/VoltStaff/VoltStaff.h"

// Tキーでタイトルへ戻る。
void GameScene::Event()
{
	if (GetAsyncKeyState('T') & 0x8000)
	{
		SceneManager::Instance().SetNextScene
		(
			SceneManager::SceneType::Title
		);
	}
}

// ゲームシーンで使うオブジェクトを作成し、参照関係をつなぐ。
void GameScene::Init()
{
	// 村の中で復活する位置。
	const Math::Vector3 playerRespawnPos = { -30.0f, 0.0f, 0.0f };

	// カメラを作成する。
	std::shared_ptr<TPSCamera> camera;
	camera = std::make_shared<TPSCamera>();
	camera->Init();
	camera->SetYawDeg(-90.0f);
	m_objList.push_back(camera);

	// プレイヤーを作成する。
	std::shared_ptr<Player> player;
	player = std::make_shared<Player>();
	player->SetAngle(DirectX::XMConvertToRadians(-90.0f));
	m_objList.push_back(player);

	// ステータスUIを作成する。
	std::shared_ptr<Status> status;
	status = std::make_shared<Status>();
	m_status = status;
	m_objList.push_back(status);

	// コウモリの生成管理を作成する。
	std::shared_ptr<BatGroup> batGroup;
	batGroup = std::make_shared<BatGroup>();
	m_objList.push_back(batGroup);
	batGroup->AddHabitat({ -70.0f, 3.0f,   0.0f }, 16.0f, 15);
	batGroup->AddHabitat({ -30.0f, 3.0f,  55.0f }, 14.0f, 14);
	batGroup->AddHabitat({ -30.0f, 3.0f, -75.0f }, 14.0f, 14);
	batGroup->AddHabitat({  55.0f, 3.0f, -45.0f }, 14.0f, 14);
	batGroup->AddHabitat({ -85.0f, 3.0f,  45.0f }, 14.0f, 14);
	batGroup->AddHabitat({ -85.0f, 3.0f, -45.0f }, 14.0f, 14);
	batGroup->AddHabitat({ -120.0f, 3.0f,  0.0f }, 16.0f, 15);
	batGroup->SetStatus(status);

	// 地面を作成する。
	std::shared_ptr<Ground> ground;
	ground = std::make_shared<Ground>();
	m_objList.push_back(ground);

	// 村を作成する。
	std::shared_ptr<Village> village;
	village = std::make_shared<Village>();
	m_objList.push_back(village);

	// 杖を作成する。
	std::shared_ptr<FireStaff> fireStaff;
	fireStaff = std::make_shared<FireStaff>();
	m_objList.push_back(fireStaff);

	std::shared_ptr<IceStaff> iceStaff;
	iceStaff = std::make_shared<IceStaff>();
	m_objList.push_back(iceStaff);

	std::shared_ptr<VoltStaff> voltStaff;
	voltStaff = std::make_shared<VoltStaff>();
	m_objList.push_back(voltStaff);

	// 各オブジェクト同士の参照をつなぐ。
	camera->SetTarget(player);
	player->SetStatus(status);
	player->SetCamera(camera);
	player->SetRespawnPos(playerRespawnPos);
	player->SetSafeArea(village->GetSafeAreaCenter(), village->GetSafeAreaRadius());
	ground->SetTarget(player);
	village->SetTarget(player);
	village->SetVisibleRadius(85.0f);
	batGroup->SetSafeArea(village->GetSafeAreaCenter(), village->GetSafeAreaRadius());
	batGroup->AddBatsToScene(m_objList, player);
	fireStaff->SetTarget(player);
	iceStaff->SetTarget(player);
	voltStaff->SetTarget(player);
	fireStaff->SetStatus(status);
	iceStaff->SetStatus(status);
	voltStaff->SetStatus(status);
	
	fireStaff->SetAngle(0.0f);
	iceStaff->SetAngle(DirectX::XMConvertToRadians(120.0f));
	voltStaff->SetAngle(DirectX::XMConvertToRadians(240.0f));

	// プレイヤーの地形判定対象を登録する。
	player->RegistHitObject(ground);
	player->RegistHitObject(village);

	status->SetPlayer(player);
}

bool GameScene::IsUpdatePaused() const
{
	std::shared_ptr<Status> spStatus = m_status.lock();
	if (!spStatus) { return false; }

	return spStatus->IsLevelUpSelect();
}

bool GameScene::CanUpdateWhenPaused(const std::shared_ptr<KdGameObject>& obj) const
{
	return std::dynamic_pointer_cast<Status>(obj) != nullptr;
}



















