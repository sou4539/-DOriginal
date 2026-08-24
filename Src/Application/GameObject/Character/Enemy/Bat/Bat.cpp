#include "Bat.h"

#include "../../Player/Player.h"
#include "../../Status/Status.h"

namespace
{
	// �����̃R�E�����܂Ŗ��t���[�����A�j���[�V������i�߂�ƁA
	constexpr float BatAnimActiveRadius = 45.0f;
	constexpr int BatHitFlashFrame = 5;
	const Math::Color BatHitColor = Math::Color(2.0f, 0.05f, 0.05f, 1.0f);
	const Math::Vector3 BatHitEmissive = Math::Vector3(1.2f, 0.0f, 0.0f);
}

void Bat::Init()
{

	//�R�E�������f����ǂݍ��ށB

	if (!m_spModel)
	{
		m_spModel = std::make_shared<KdModelWork>();
		m_spModel->SetModelData("Asset/Models/Objects/Character/Bat/Bat.gltf");
	}

	// Play bat flap animation.
	if (m_spModel)
	{
		m_animator.SetAnimation(m_spModel->GetAnimation("flap_loop"), true);
	}


	// Set temporary initial position.
	m_pos = { -15,3,0 };
	m_startPos = m_pos;
	SetPos(m_pos);

	// ���m�͈͂��f�o�b�O�\�����邽�߂̃��C�����쐬����B
	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

	// �R�E�����Ƀ_���[�W�������������B
	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape
	(
		"BatDamage",
		Math::Vector3::Zero,
		m_damageRadius,
		KdCollider::TypeDamage
	);
}

void Bat::Update()
{
	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
	float distanceSqr = 0.0f;
	bool hasTargetDistance = false;
	bool isMoving = false;

	if (spTarget)
	{
		Math::Vector3 toTarget = spTarget->GetPos() - m_pos;
		distanceSqr = toTarget.LengthSquared();
		hasTargetDistance = true;
	}

	if (m_hitFlashFrame > 0)
	{
		--m_hitFlashFrame;
	}

	// �R�E�����̍��G�͈͂��f�o�b�O���C���ŕ\������B
	if (m_pDebugWire)
	{
		float debugRadius = m_isChasing ? m_chaseRadius : m_searchRadius;
		m_pDebugWire->AddDebugSphere(m_pos, debugRadius, kRedColor);
	}

	if (spTarget)
	{
		// �v���C���[�����̈��S�n�тɂ��邩�m�F����B
		bool isTargetInSafeArea = false;
		std::shared_ptr<Player> spPlayer = std::dynamic_pointer_cast<Player>(spTarget);
		if (spPlayer)
		{
			isTargetInSafeArea = spPlayer->IsInSafeArea();
		}

		// �v���C���[�܂ł̕����Ƌ����𒲂ׂ�B
		Math::Vector3 toTarget = spTarget->GetPos() - m_pos;
		distanceSqr = toTarget.LengthSquared();

		// ���̈��S�n�тɓ�������A�ǐՏ�Ԃ���������B
		if (isTargetInSafeArea)
		{
			m_isChasing = false;
		}
		else
		{
			// ������O�͏����������͈͂Ŕ��肷��B
			if (!m_isChasing && distanceSqr <= m_searchRadius * m_searchRadius)
			{
				m_isChasing = true;
			}
			else if (m_isChasing && distanceSqr > m_chaseRadius * m_chaseRadius)
			{
				m_isChasing = false;
			}
		}

		// �ǐՏ�ԂȂ�v���C���[�ֈړ�����B
		if (m_isChasing)
		{
			// ������0�ɋ߂��Ɛ��K���ł��Ȃ����߁A��������Ă��鎞���������B
			if (distanceSqr > 0.0001f)
			{
				toTarget.Normalize();

				// �v���C���[��菭���x�����x�ŋ߂Â��B
				m_pos += toTarget * m_moveSpeed;
				isMoving = true;

				// �ړ������ɍ��킹�ăR�E�����̌�����ς���B
				m_angle = atan2(toTarget.x, toTarget.z);
			}
		}
		else
		{
			// �v���C���[�����m�X�t�B�A�̊O�ɂ���A�܂��͈��S�n�тɂ���ꍇ�́A
			Math::Vector3 toStart = m_startPos - m_pos;
			float startDistanceSqr = toStart.LengthSquared();
			float moveSpeedSqr = m_moveSpeed * m_moveSpeed;

			// �����ʒu�܂ł̋�����1�t���[���̈ړ��ʈȉ��Ȃ�A
			if (startDistanceSqr <= moveSpeedSqr)
			{
				isMoving = (startDistanceSqr > 0.0001f);
				m_pos = m_startPos;
			}
			else
			{
				toStart.Normalize();
				m_pos += toStart * m_moveSpeed;
				isMoving = true;

				// �߂鎞���ړ������Ɍ��������킹��B
				m_angle = atan2(toStart.x, toStart.z);
			}
		}
	}

	//�R�E�����S�̂̃��[���h�s������B

	Math::Matrix scaleMat = Math::Matrix::CreateScale(0.5);
	// Bat���f���̐��ʕ������ړ������̌v�Z�Ƌt�����Ȃ̂ŁA
	Math::Matrix rotMat = Math::Matrix::CreateRotationY(m_angle + DirectX::XM_PI);
	Math::Matrix transMat = Math::Matrix::CreateTranslation(m_pos);
	m_mWorld = scaleMat * rotMat * transMat;

	const float animActiveRadiusSqr = BatAnimActiveRadius * BatAnimActiveRadius;
	if (m_spModel && (!hasTargetDistance || isMoving || m_isChasing || distanceSqr <= animActiveRadiusSqr))
	{
		m_animator.AdvanceTime(m_spModel->WorkNodes(), 1.0f);
	}
}

void Bat::DrawLit()
{
	if (!m_spModel) { return; }

	if (m_hitFlashFrame > 0)
	{
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld, BatHitColor, BatHitEmissive);
		return;
	}

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld, kWhiteColor);

}

void Bat::GenerateDepthMapFromLight()
{
	if (!m_spModel) { return; }

	// Draw animated bat model into the shadow map.
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

void Bat::DrawEffect()
{
	// Bat uses the real shadow map, so no fake flattened shadow is drawn here.
}

void Bat::OnHit()
{
	// �����Ȃ��ŌĂ΂ꂽ�ꍇ�́A���̊�{�_���[�W���g���B
	OnHit(10.0f);
}

void Bat::OnHit(float damage)
{
	// ���łɎ��S�����������Ă���ꍇ�́A��d�Ɍo���l������Ȃ��悤�ɉ������Ȃ��B
	if (m_isExpired) { return; }

	// 0�ȉ��̃_���[�W�͖����ɂ���B
	if (damage <= 0.0f) { return; }

	// ���@�����������̂ŁA�󂯎�����_���[�W�ʂ���HP�����炷�B
	m_hp -= damage;
	m_hitFlashFrame = BatHitFlashFrame;

	// HP��0�ȉ��ɂȂ�����ABaseScene::PreUpdate�ō폜�����悤�ɂ���B
	if (m_hp <= 0.0f)
	{
		m_isExpired = true;

		// �R�E������|������V�Ƃ��āA�v���C���[��Status�֌o���l��n���B
		std::shared_ptr<Status> spStatus = m_wpStatus.lock();
		if (spStatus)
		{
			spStatus->AddExp(m_exp);
		}
	}
}









