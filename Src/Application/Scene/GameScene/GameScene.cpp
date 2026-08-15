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

// GameScene中の入力イベント処理。
// 使い方：
//   BaseScene側から毎フレーム呼ばれる。
// 処理内容：
//   今はデバッグ用として、Tキーを押したらタイトルシーンへ戻る。
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

// GameSceneの初期化処理。
// 使い方：
//   GameScene生成時にコンストラクタから自動で呼ばれる。
// 処理内容：
//   ゲームで使うカメラ、プレイヤー、コウモリ、UI、地面、村を作成してm_objListに登録する。
//   その後、カメラの追従対象、プレイヤーのStatus、復活地点、当たり判定対象を設定する。
void GameScene::Init()
{
	// 村の中で復活する位置。
	// 村の配置を変えた時は、Player.cppではなくこの値を調整する。
	const Math::Vector3 playerRespawnPos = { -30.0f, 0.0f, 0.0f };

	// TPSカメラを作成する。
	// プレイヤーを追従対象にするため、あとでSetTargetする。
	std::shared_ptr<TPSCamera> camera;
	camera = std::make_shared<TPSCamera>();
	camera->Init();
	m_objList.push_back(camera);

	// プレイヤーを作成する。
	std::shared_ptr<Player> player;
	player = std::make_shared<Player>();
	m_objList.push_back(player);

	// コウモリの生息地を管理するBatGroupを作成する。
	// 生息地ごとに白いスフィア範囲を持ち、その中へコウモリをランダム配置する。
	std::shared_ptr<BatGroup> batGroup;
	batGroup = std::make_shared<BatGroup>();
	m_objList.push_back(batGroup);
	batGroup->AddHabitat({ -70.0f, 3.0f,   0.0f }, 12.0f, 4);
	batGroup->AddHabitat({ -45.0f, 3.0f,  35.0f }, 10.0f, 3);
	batGroup->AddHabitat({ -95.0f, 3.0f, -35.0f }, 14.0f, 5);
	batGroup->AddBatsToScene(m_objList, player);

	// ステータスUI/情報管理用のオブジェクトを作成する。
	std::shared_ptr<Status> status;
	status = std::make_shared<Status>();
	m_objList.push_back(status);

	// 広い地面を作成する。
	// プレイヤーの地面判定対象にも登録する。
	std::shared_ptr<Ground> ground;
	ground = std::make_shared<Ground>();
	m_objList.push_back(ground);

	// 村モデルを作成する。
	// プレイヤーの壁・地面判定対象にも登録する。
	std::shared_ptr<Village> village;
	village = std::make_shared<Village>();
	m_objList.push_back(village);

	//杖モデルを作成する
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
	// ここでつないでおくことで、Update内で相手の情報を使える。
	camera->SetTarget(player);
	player->SetStatus(status);
	player->SetCamera(camera);
	player->SetRespawnPos(playerRespawnPos);
	player->SetSafeArea(village->GetSafeAreaCenter(), village->GetSafeAreaRadius());
	fireStaff->SetTarget(player);
	iceStaff->SetTarget(player);
	voltStaff->SetTarget(player);
	
	fireStaff->SetAngle(0.0f);
	iceStaff->SetAngle(DirectX::XMConvertToRadians(120.0f));
	voltStaff->SetAngle(DirectX::XMConvertToRadians(240.0f));

	// プレイヤーが当たり判定を調べる対象を登録する。
	player->RegistHitObject(ground);
	player->RegistHitObject(village);

	status->SetPlayer(player);
}



