#pragma once
#include "../CharaBase.h"

class Status;
class CameraBase;

class Player : public CharaBase
{
public:
	// Player���쐬�������ɁA������Init()���Ă�ŏ���������B
	Player() { Init(); }

	// Player�j�����̏����B
	~Player() override {}

	// ���t���[���̒ʏ�X�V�B
	void Update() override;

	// 影を落とすため、ライト視点の深度マップへプレイヤーモデルを描画する。
	void GenerateDepthMapFromLight() override;

	// 地面に接地感を出すため、簡易的な黒い影を描画する。
	void DrawEffect() override;

	// Update��ɌĂ΂��X�V�B
	void PostUpdate() override;

	// Player���_���[�W���󂯂����ɑ��삷��Status��o�^����B
	void SetStatus(const std::shared_ptr<Status>& status)
	{
		m_status = status;
	}

	// �J������ňړ����邽�߂ɁA���ݎg���Ă���J������o�^����B
	void SetCamera(const std::shared_ptr<CameraBase>& camera)
	{
		m_wpCamera = camera;
	}

	// HP��0�ɂȂ������ɖ߂���W���O����ݒ肷��B
	void SetRespawnPos(const Math::Vector3& respawnPos)
	{
		m_respawnPos = respawnPos;
	}

	// ���̈��S�n�уX�t�B�A��ݒ肷��B
	void SetSafeArea(const Math::Vector3& center, float radius)
	{
		m_safeAreaCenter = center;
		m_safeAreaRadius = radius;

		Math::Vector3 toPlayer = GetPos() - m_safeAreaCenter;
		toPlayer.y = 0.0f;
		m_isInSafeArea = toPlayer.LengthSquared() <= m_safeAreaRadius * m_safeAreaRadius;
	}

	// �v���C���[�����S�n�тɂ��邩��Ԃ��B
	bool IsInSafeArea() const { return m_isInSafeArea; }

	// �v���C���[����̗L��/������؂�ւ���B
	void SetControlEnable(bool enable) { m_isControlEnable = enable; }

	// �O������\���ʒu��ݒ肷��B
	void SetPos(const Math::Vector3& pos) override
	{
		m_pos = pos;
		KdGameObject::SetPos(pos);
	}

	// �^�C�g����ʂȂǂŁA�v���C���[�̌����������w�肵�������Ɏg���B
	void SetAngle(float angle) { m_angle = angle; }

private:
	// Player�̏����������B
	void Init() override;

	// ���G���Ԃ��X�V����B
	void UpdateInvincible();

	// ���͂ƃJ������������ړ����������A�v���C���[���ړ�������B
	void UpdateMove();

	// m_pos��m_angle����A�`��p�̃��[���h�s������B
	void UpdateWorldMatrix();

	// �v���C���[�̗̑p�X�t�B�A���ATypeDamage�̓����蔻��ɐG��Ă��邩�m�F����B
	void UpdateDamageCollision();

	// HP��0�ɂȂ��Ă��邩�m�F���A0�Ȃ瑺�̕����n�_�֖߂��B
	void RespawnIfDead();

	// �v���C���[�����̈��S�n�ѓ��ɂ��邩�m�F����B
	void UpdateSafeAreaFlag();

	// �v���C���[HP��Status���Ǘ����Ă���B
	std::weak_ptr<Status> m_status;

	// WASD���͂��J������̈ړ������֕ϊ����邽�߂Ɏg���B
	std::weak_ptr<CameraBase> m_wpCamera;

	// �v���C���[��Y����]�p�x�B
	float m_angle = 0.0f;

	// �_���[�W���󂯂���̖��G���ԁB
	float m_damageCoolTime = 0.0f;

	// HP��0�ɂȂ������ɖ߂鑺�̒��̍��W�B
	Math::Vector3 m_respawnPos = Math::Vector3::Zero;

	// ���̈��S�n�тɂ��邩�ǂ����B
	bool m_isInSafeArea = false;

	// ���𕢂����S�n�уX�t�B�A�B
	Math::Vector3 m_safeAreaCenter = Math::Vector3::Zero;
	float m_safeAreaRadius = 0.0f;

	// true�Ȃ�WASD���͂ňړ�����B
	bool m_isControlEnable = true;
};





