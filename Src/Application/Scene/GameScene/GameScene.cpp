#include "GameScene.h"
#include"../SceneManager.h"

#include "../../GameObject/Camera/TPSCamera/TPSCamera.h"
#include "../../GameObject/Character/Player/Player.h"
//#include "../../GameObject/Character/Enemy/Enemy.h"
#include "../../GameObject/Character/Status/Status.h"

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
	std::shared_ptr<TPSCamera> camera = std::make_shared<TPSCamera>();
	m_objList.push_back(camera);

	std::shared_ptr<Player> player = std::make_shared<Player>();
	m_objList.push_back(player);

	std::shared_ptr<Status> status = std::make_shared<Status>();
	m_objList.push_back(status);

	//camera->SetTarget(player);
	player->SetStatus(status);
	status->SetPlayer(player);
}
