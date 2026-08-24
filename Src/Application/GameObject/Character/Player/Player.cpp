#include "Player.h"

#include "../../Camera/CameraBase.h"
#include "../Status/Status.h"
#include "../../../Scene/SceneManager.h"

namespace
{
	constexpr float PlayerMoveSpeed = 0.1f;
	constexpr float PlayerDamageRadius = 1.2f;
	constexpr float PlayerDamageSphereHeight = 1.5f;
	constexpr float BatContactDamage = 5.0f;
	constexpr float DamageCoolTimeFrame = 60.0f;
	constexpr float RespawnInvincibleFrame = 120.0f;

	const Math::Vector3 DefaultRespawnPos = { -30.0f, 0.0f, 0.0f };
}

// Player�̏����������B
void Player::Init()
{
	// �v���C���[���f����ǂݍ��ށB
	if (!m_spModel)
	{
		m_spModel = std::make_shared<KdModelWork>();
		m_spModel->SetModelData("Asset/Models/Objects/Character/Witch/Witch.gltf");
	}

	// �v���C���[�̏����ʒu�B
	m_respawnPos = DefaultRespawnPos;
	m_pos = m_respawnPos;

	// KdGameObject���̃��[���h�s��ɂ������ʒu�𔽉f����B
	SetPos(m_pos);
}

void Player::GenerateDepthMapFromLight()
{
	if (!m_spModel) { return; }

	// 通常描画と同じモデル行列で描くことで、見た目と同じ形の影を作る。
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}
void Player::DrawEffect()
{
	if (!m_spModel) { return; }

	// Fake ground shadow.
	Math::Matrix shadowMat = m_mWorld * Math::Matrix::CreateScale(1.0f, 0.0f, 1.0f);
	shadowMat.Translation({ m_pos.x, 0.03f, m_pos.z });

	const Math::Color shadowColor = { 0.0f, 0.0f, 0.0f, 0.35f };
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, shadowMat, shadowColor);
}
// Player�̖��t���[���X�V�B
void Player::Update()
{
	// CharaBase���̊�{�X�V���ĂԁB
	CharaBase::Update();

	UpdateInvincible();
	if (m_isControlEnable)
	{
		UpdateMove();
	}
	UpdateWorldMatrix();
}

// ���G���Ԃ̍X�V�����B
void Player::UpdateInvincible()
{
	if (m_damageCoolTime <= 0.0f) { return; }

	// ���G���Ԃ�1�t���[�������炷�B
	m_damageCoolTime -= 1.0f;
}

// �v���C���[�̈ړ������B
void Player::UpdateMove()
{
	// WASD���͂���ړ����������������B
	Math::Vector3 moveDir = Math::Vector3::Zero;

	if (GetAsyncKeyState('W') & 0x8000)
	{
		moveDir.z += 1.0f;
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		moveDir.z -= 1.0f;
	}
	if (GetAsyncKeyState('A') & 0x8000)
	{
		moveDir.x -= 1.0f;
	}
	if (GetAsyncKeyState('D') & 0x8000)
	{
		moveDir.x += 1.0f;
	}

	if (moveDir.LengthSquared() > 0.0f)
	{
		// �΂߈ړ����ɑ��x�������Ȃ�Ȃ��悤�A�����x�N�g���𐳋K������B
		moveDir.Normalize();

		// �J�������Ȃ��ꍇ�́A���͕��������̂܂܈ړ������Ƃ��Ďg���B
		m_dir = moveDir;

		std::shared_ptr<CameraBase> spCamera = m_wpCamera.lock();
		if (spCamera)
		{
			// �J������Y��]�������g���A���͕������J������̕����֕ϊ�����B
			m_dir = Math::Vector3::TransformNormal(moveDir, spCamera->GetRotationYMatrix());
			m_dir.Normalize();
		}

		// ���ۂɃv���C���[���W���ړ�������B
		m_pos += m_dir * PlayerMoveSpeed;

		// �ړ���������Y����]�p�x�����B
		m_angle = atan2(m_dir.x, m_dir.z);
	}
}

// �v���C���[�̃��[���h�s�����鏈���B
void Player::UpdateWorldMatrix()
{
	// �v���C���[�̃��[���h�s������B
	Math::Matrix m_scale = Math::Matrix::CreateScale(1);
	Math::Matrix m_rot = Math::Matrix::CreateRotationY(m_angle);
	Math::Matrix m_trans = Math::Matrix::CreateTranslation(m_pos);
	m_mWorld = m_scale * m_rot * m_trans;
}

// Update��̕␳�E���菈���B
void Player::PostUpdate()
{
	// CharaBase���Œn�ʂ�ǂƂ̓����蔻����s���B
	CharaBase::PostUpdate();

	// CharaBase�̓����蔻��ŕ␳���ꂽ���W���APlayer����m_pos�ɂ����f����B
	m_pos = GetPos();

	// ���݈ʒu�����̈��S�n�ѓ����ǂ������X�V����B
	UpdateSafeAreaFlag();

	// �ړ��ƒn�`�␳���I�������̐��������W�ŁA
	UpdateDamageCollision();

	// �_���[�W����̌���HP��0�ɂȂ����ꍇ�́A�^�C�g���֖߂炸���̒��ŕ�������B
	RespawnIfDead();
}

// �G�Ƃ̐ڐG�_���[�W����B
void Player::UpdateDamageCollision()
{
	// ���S�n�ѓ��ł͓G�Ƃ̐ڐG�_���[�W���󂯂Ȃ��B
	if (m_isInSafeArea) { return; }

	// ���G���Ԓ��̓_���[�W���󂯂Ȃ��B
	if (m_damageCoolTime > 0.0f) { return; }

	// HP��Status�������Ă��邽�߁A�܂�Status���擾����B
	std::shared_ptr<Status> spStatus = m_status.lock();
	if (!spStatus) { return; }

	// �v���C���[�̑̂����Ƃ��Ĉ����B
	DirectX::BoundingSphere playerSphere;
	playerSphere.Center = GetPos() + Math::Vector3(0.0f, PlayerDamageSphereHeight, 0.0f);
	playerSphere.Radius = PlayerDamageRadius;

	// TypeDamage����������SphereInfo�����B
	KdCollider::SphereInfo sphereInfo(KdCollider::TypeDamage, playerSphere);

	// ���݂̃V�[���ɑ��݂���S�I�u�W�F�N�g�𒲂ׂ�B
	const std::list<std::shared_ptr<KdGameObject>>& objList = SceneManager::Instance().GetObjList();
	for (const std::shared_ptr<KdGameObject>& spObj : objList)
	{
		// ��̃|�C���^�͖�������B
		if (!spObj) { continue; }

		// �������g�Ƃ͔��肵�Ȃ��B
		if (spObj.get() == this) { continue; }

		// �ΏۃI�u�W�F�N�g��TypeDamage�̃R���C�_�[�������Ă��āA
		std::list<KdCollider::CollisionResult> retList;
		if (spObj->Intersects(sphereInfo, &retList))
		{
			// �_���[�W����ɐG�ꂽ�̂ŁA�v���C���[HP��5���炷�B
			spStatus->DamagePlayer(BatContactDamage);

			// ���̃_���[�W�܂Ŗ�1�b�҂B
			m_damageCoolTime = DamageCoolTimeFrame;

			// 1�̂ł��������Ă���΁A����̃_���[�W�����͏I���B
			break;
		}
	}
}

// HP��0�ɂȂ������̕��������B
void Player::RespawnIfDead()
{
	// HP��Status���ŊǗ����Ă��邽�߁A�܂�Status���擾����B
	std::shared_ptr<Status> spStatus = m_status.lock();
	if (!spStatus) { return; }

	// HP���܂��c���Ă���Ȃ畜�������͕s�v�B
	if (!spStatus->IsPlayerDead()) { return; }

	// �v���C���[�𑺂̕����n�_�֖߂��B
	m_pos = m_respawnPos;
	SetPos(m_respawnPos);

	// ��������̃��[���h�s��������������ʒu�ɂ��Ă����B
	UpdateWorldMatrix();

	// HP���ő�܂ŉ񕜂���B
	spStatus->ResetPlayerHp();

	// ��������ɃR�E�����֐G��Ă��Ă��A�����ă_���[�W���󂯂Ȃ��悤�ɂ���B
	m_damageCoolTime = RespawnInvincibleFrame;
}

void Player::UpdateSafeAreaFlag()
{
	// ���a��0�ȉ��Ȃ�A���S�n�т����ݒ�Ȃ̂�false�ɂ���B
	if (m_safeAreaRadius <= 0.0f)
	{
		m_isInSafeArea = false;
		return;
	}

	// XZ���ʏ�ŁA�v���C���[�����̈��S�n�уX�t�B�A���ɂ��邩�m�F����B
	Math::Vector3 toPlayer = GetPos() - m_safeAreaCenter;
	toPlayer.y = 0.0f;

	float distanceSqr = toPlayer.LengthSquared();
	m_isInSafeArea = distanceSqr <= m_safeAreaRadius * m_safeAreaRadius;
}



