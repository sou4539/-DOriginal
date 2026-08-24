#include "TPSCamera.h"

namespace
{
	constexpr float CameraLookAheadDistance = 5.0f;
}

void TPSCamera::Init()
{
	// �e�N���X�̏������Ăяo��
	CameraBase::Init();

	// �v���C���[���猩���J�����̑��Έʒu�B
	m_mLocalPos = Math::Matrix::CreateTranslation(0, 15.0f, -15.0f);

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);
}

void TPSCamera::PostUpdate()
{
	// �^�[�Q�b�g�̍��W(�L���ȏꍇ���p����)
	Math::Vector3								_targetPos = Math::Vector3::Zero;
	const std::shared_ptr<const KdGameObject>	_spTarget = m_wpTarget.lock();
	if (_spTarget)
	{
		_targetPos = _spTarget->GetPos();
		// �����ł͂Ȃ��A�v���C���[�̑̂̒��S�����������B
		_targetPos.y += 1.0f;
	}

	// �J�����̉�]
	UpdateRotateByMouse();

	// �}�E�X���E�̉�]�������g���āA�v���C���[�̎��͂���荞�ށB
	m_mRotation = GetRotationYMatrix();

	Math::Vector3 _lookTargetPos = _targetPos;
	if (_spTarget)
	{
		// プレイヤーより少し前を見ることで、進行方向を広めに映す。
		Math::Vector3 _lookAheadDir = Math::Vector3::TransformNormal(Math::Vector3(0.0f, 0.0f, 1.0f), m_mRotation);
		_lookAheadDir.y = 0.0f;
		if (_lookAheadDir.LengthSquared() > 0.0f)
		{
			_lookAheadDir.Normalize();
			_lookTargetPos += _lookAheadDir * CameraLookAheadDistance;
		}
	}

	// �v���C���[���猩���J�����܂ł̋����B
	Math::Vector3 _cameraDistance = Math::Vector3(0, 15.0f, -15.0f);

	// �J�����܂ł̋������A�}�E�X���E�̉�]�ɍ��킹�ĉ񂷁B
	Math::Vector3 _cameraOffset = Math::Vector3::TransformNormal(_cameraDistance, m_mRotation);
	Math::Vector3 _cameraPos = _targetPos + _cameraOffset;

	// �J��������v���C���[�֌����������B
	Math::Vector3 _toTarget = _lookTargetPos - _cameraPos;
	_toTarget.Normalize();

	// �J�����̃��[���h�s����u���W�v�Ɓu�����v������B
	m_mWorld = Math::Matrix::CreateWorld(_cameraPos, -_toTarget, Math::Vector3::Up);

	// ���߂荞�ݖh�~�ׂ̈̍��W�␳�v�Z��
	KdCollider::RayInfo rayInfo;
	// ���C�̔��ˈʒu��ݒ�
	rayInfo.m_pos = GetPos();

	// ���C�̔��˕�����ݒ�
	rayInfo.m_dir = Math::Vector3::Down;
	// ���C�̒�����ݒ�
	rayInfo.m_range = 1000.f;
	if (_spTarget)
	{
		rayInfo.m_dir = _lookTargetPos - GetPos();
		rayInfo.m_range = rayInfo.m_dir.Length();
		rayInfo.m_dir.Normalize();
	}

	// �����蔻����������^�C�v��ݒ�
	rayInfo.m_type = KdCollider::TypeGround;

	// �AHIT����ΏۃI�u�W�F�N�g�ɑ�������
	for (std::weak_ptr<KdGameObject> wpGameObj : m_wpHitObjectList)
	{
		std::shared_ptr<KdGameObject> spGameObj = wpGameObj.lock();
		if (spGameObj)
		{
			std::list<KdCollider::CollisionResult> retRayList;
			spGameObj->Intersects(rayInfo, &retRayList);

			// �B ���ʂ��g���č��W��⊮����
			float maxOverLap = 0;
			Math::Vector3 hitPos = {};
			bool hit = false;
			for (auto& ret : retRayList)
			{
				// ���C���Ւf���I�[�o�[����������
				if (maxOverLap < ret.m_overlapDistance)
				{
					maxOverLap = ret.m_overlapDistance;
					hitPos = ret.m_hitPos;
					hit = true;
				}
			}
			if (hit)
			{
				// ��������̏�Q���ɓ������Ă���
				Math::Vector3 _hitPos = hitPos;
				_hitPos += rayInfo.m_dir * 0.4f;
				SetPos(_hitPos);
			}
		}
	}
}
