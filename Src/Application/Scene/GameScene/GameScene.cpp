#include "GameScene.h"
#include "../SceneManager.h"

#include "../../GameObject/Camera/TPSCamera/TPSCamera.h"
#include "../../GameObject/Character/Enemy/EnemySpawner/EnemySpawner.h"
#include "../../GameObject/Character/Player/Player.h"
#include "../../GameObject/Character/Status/Status.h"
#include "../../GameObject/Stage/Ground/Ground.h"
#include "../../GameObject/Stage/Village/Village.h"
#include "../../GameObject/Stage/Tree/Tree.h"
#include "../../GameObject/Character/Staff/FireStaff/FireStaff.h"
#include "../../GameObject/Character/Staff/IceStaff/IceStaff.h"
#include "../../GameObject/Character/Staff/VoltStaff/VoltStaff.h"

GameScene::~GameScene()
{
	SetCursorVisible(false);
}

void GameScene::SetCursorVisible(bool isVisible)
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

// T�L�[���������u�ԂɃ^�C�g���֖߂�B
void GameScene::Event()
{
	SetCursorVisible(false);

	const bool isBackTitleKey = (GetAsyncKeyState('T') & 0x8000);
	if (isBackTitleKey && !m_prevBackTitleKey)
	{
		SceneManager::Instance().SetNextScene
		(
			SceneManager::SceneType::Title
		);
	}
	m_prevBackTitleKey = isBackTitleKey;
}

// �Q�[���V�[���Ŏg���I�u�W�F�N�g���쐬���A�Q�Ɗ֌W���Ȃ��B
void GameScene::Init()
{
	// ���̒��ŕ�������ʒu�B
	const Math::Vector3 playerRespawnPos = { -30.0f, 0.0f, 0.0f };

	// �J�������쐬����B
	std::shared_ptr<TPSCamera> camera;
	camera = std::make_shared<TPSCamera>();
	camera->Init();
	camera->SetYawDeg(-90.0f);
	m_objList.push_back(camera);

	// �v���C���[���쐬����B
	std::shared_ptr<Player> player;
	player = std::make_shared<Player>();
	player->SetAngle(DirectX::XMConvertToRadians(-90.0f));
	m_objList.push_back(player);

	// �X�e�[�^�XUI���쐬����B
	std::shared_ptr<Status> status;
	status = std::make_shared<Status>();
	m_status = status;
	m_objList.push_back(status);

	// �G�̐����Ǘ����쐬����B
	std::shared_ptr<EnemySpawner> enemySpawner;
	enemySpawner = std::make_shared<EnemySpawner>();
	m_objList.push_back(enemySpawner);
	enemySpawner->AddSpawnArea({ -70.0f, 3.0f,   0.0f }, 16.0f, 15);
	enemySpawner->AddSpawnArea({ -30.0f, 3.0f,  55.0f }, 14.0f, 14);
	enemySpawner->AddSpawnArea({ -30.0f, 3.0f, -75.0f }, 14.0f, 14);
	enemySpawner->AddSpawnArea({  55.0f, 3.0f, -45.0f }, 14.0f, 14);
	enemySpawner->AddSpawnArea({ -85.0f, 3.0f,  45.0f }, 14.0f, 14);
	enemySpawner->AddSpawnArea({ -85.0f, 3.0f, -45.0f }, 14.0f, 14);
	enemySpawner->AddSpawnArea({ -120.0f, 3.0f,  0.0f }, 16.0f, 15);
	enemySpawner->SetStatus(status);

	// �n�ʂ��쐬����B
	std::shared_ptr<Ground> ground;
	ground = std::make_shared<Ground>();
	m_objList.push_back(ground);

	// �����쐬����B
	std::shared_ptr<Village> village;
	village = std::make_shared<Village>();
	m_objList.push_back(village);

	std::shared_ptr<Tree> tree;
	tree = std::make_shared<Tree>();
	m_objList.push_back(tree);

	// ����쐬����B
	std::shared_ptr<FireStaff> fireStaff;
	fireStaff = std::make_shared<FireStaff>();
	m_objList.push_back(fireStaff);

	std::shared_ptr<IceStaff> iceStaff;
	iceStaff = std::make_shared<IceStaff>();
	m_objList.push_back(iceStaff);

	std::shared_ptr<VoltStaff> voltStaff;
	voltStaff = std::make_shared<VoltStaff>();
	m_objList.push_back(voltStaff);

	// �e�I�u�W�F�N�g���m�̎Q�Ƃ��Ȃ��B
	camera->SetTarget(player);
	// ����̖��@�I���ŃQ�[���X�V���~�܂��Ă��A�J���������͐����������ʒu�ɂ��Ă����B
	camera->PostUpdate();
	player->SetStatus(status);
	player->SetCamera(camera);
	player->SetRespawnPos(playerRespawnPos);
	player->SetSafeArea(village->GetSafeAreaCenter(), village->GetSafeAreaRadius());
	ground->SetTarget(player);
	village->SetTarget(player);
	village->SetVisibleRadius(85.0f);
	enemySpawner->SetSafeArea(village->GetSafeAreaCenter(), village->GetSafeAreaRadius());
	enemySpawner->AddEnemiesToScene(m_objList, player);
	status->SetVillageGuideRadius(village->GetVisibleRadius());
	status->SetCamera(camera);
	fireStaff->SetTarget(player);
	iceStaff->SetTarget(player);
	voltStaff->SetTarget(player);
	fireStaff->SetStatus(status);
	iceStaff->SetStatus(status);
	voltStaff->SetStatus(status);

	// �v���C���[�̒n�`����Ώۂ�o�^����B
	player->RegistHitObject(ground);
	player->RegistHitObject(village);
	player->RegistHitObject(tree);

	status->SetPlayer(player);

	SetCursorVisible(false);
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
