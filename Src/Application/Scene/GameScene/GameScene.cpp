#include "GameScene.h"
#include"../SceneManager.h"

#include "../../GameObject/Camera/TPSCamera/TPSCamera.h"
#include "../../GameObject/Character/Bat/Bat.h"
#include "../../GameObject/Character/Player/Player.h"
//#include "../../GameObject/Character/Enemy/Enemy.h"
#include "../../GameObject/Character/Status/Status.h"
#include "../../GameObject/Stage/Ground/Ground.h"
#include "../../GameObject/Stage/Village/Village.h"

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

void GameScene::Init()
{
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

	// コウモリを作成する。
	// 現在は表示確認用として、プレイヤーと同じ初期座標に配置している。
	std::shared_ptr<Bat> bat;
	bat = std::make_shared<Bat>();
	m_objList.push_back(bat);

	// ステータスUI/情報管理用のオブジェクトを作成する。
	std::shared_ptr<Status> status;
	status = std::make_shared<Status>();
	m_objList.push_back(status);

	// 広い地面を作成する。
	// プレイヤーの地面判定対象にも登録する。
	std::shared_ptr<Ground> ground;
	ground = std::make_shared<Ground>();
	m_objList.push_back(ground);

	// 村モデルを作成する
	std::shared_ptr<Village> village;
	village = std::make_shared<Village>();
	m_objList.push_back(village);

	// 各オブジェクト同士の参照をつなぐ。
	// ここでつないでおくことで、Update内で相手の情報を使える。
	camera->SetTarget(player);
	player->SetStatus(status);
	player->SetCamera(camera);

	// プレイヤーが当たり判定を調べる対象を登録する。
	player->RegistHitObject(ground);
	player->RegistHitObject(village);

	status->SetPlayer(player);
}
