#include "GameStageBase.h"

#include "../SceneManager.h"
#include "../../GameObject/Camera/TPSCamera/TPSCamera.h"
#include "../../GameObject/Character/Player/Player.h"
#include "../../GameObject/Character/Staff/FireStaff/FireStaff.h"
#include "../../GameObject/Character/Staff/IceStaff/IceStaff.h"
#include "../../GameObject/Character/Staff/VoltStaff/VoltStaff.h"
#include "../../GameObject/Character/Status/Status.h"


GameStageBase::~GameStageBase()
{
	SetCursorVisible(false);
}

void GameStageBase::Event()
{
	const bool cursorDown = (GetAsyncKeyState(VK_F1) & 0x8000);
	if (cursorDown && !m_prevCursorDown)
	{
		const bool isCursorUnlocked = !m_isCursorVisible;
		SetCursorVisible(isCursorUnlocked);

		for (const std::shared_ptr<KdGameObject>& obj : m_objList)
		{
			std::shared_ptr<TPSCamera> camera = std::dynamic_pointer_cast<TPSCamera>(obj);
			if (!camera) { continue; }

			camera->SetMouseLocked(!isCursorUnlocked);
			break;
		}
	}
	m_prevCursorDown = cursorDown;

	const bool titleDown = (GetAsyncKeyState('T') & 0x8000);
	if (titleDown && !m_prevTitleDown)
	{
		SceneManager::Instance().SetNextScene
		(
			SceneManager::SceneType::Title
		);
	}
	m_prevTitleDown = titleDown;
}

void GameStageBase::Start(const std::shared_ptr<Status>& status)
{
	if (m_started || !status) { return; }
	m_started = true;
	m_status = status;
	Init();
}

void GameStageBase::Init()
{
	// カメラを作成する。
	std::shared_ptr<TPSCamera> camera;
	camera = std::make_shared<TPSCamera>();
	camera->Init();
	camera->SetYawDeg(-90.0f);
	m_objList.push_back(camera);
	m_camera = camera;

	// プレイヤーを作成する。
	std::shared_ptr<Player> player;
	player = std::make_shared<Player>();
	player->SetAngle(DirectX::XMConvertToRadians(-90.0f));
	m_objList.push_back(player);
	m_player = player;

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

	// GameSceneが保持するStatusを使い、ステージ移動で能力値を失わないようにする。
	auto status = m_status.lock();
	m_objList.push_back(status);

	// ステータスの参照
	player->SetStatus(status);
	fireStaff->SetStatus(status);
	iceStaff->SetStatus(status);
	voltStaff->SetStatus(status);


	// プレイヤーの参照
	camera->SetTarget(player);
	status->SetPlayer(player);
	fireStaff->SetTarget(player);
	iceStaff->SetTarget(player);
	voltStaff->SetTarget(player);

	// カメラの参照
	player->SetCamera(camera);
	status->SetCamera(camera);

	// 参照をつないでから初期カメラ位置を更新する。
	camera->PostUpdate();
	m_prevCursorDown = (GetAsyncKeyState(VK_F1) & 0x8000) != 0;
	m_prevTitleDown = (GetAsyncKeyState('T') & 0x8000) != 0;
	SetCursorVisible(false);
}

void GameStageBase::SetCursorVisible(bool isVisible)
{
	if (m_isCursorVisible == isVisible) { return; }

	m_isCursorVisible = isVisible;

	if (isVisible)
	{
		while (ShowCursor(TRUE) < 0) {}
	}
	else
	{
		while (ShowCursor(FALSE) >= 0) {}
	}
}

bool GameStageBase::IsUpdatePaused() const
{
	std::shared_ptr<Status> status = m_status.lock();
	if (!status) { return false; }

	return status->IsLevelUpSelect();
}

bool GameStageBase::CanUpdateWhenPaused(const std::shared_ptr<KdGameObject>& obj) const
{
	return std::dynamic_pointer_cast<Status>(obj) != nullptr;
}
