#include "StaffBase.h"

#include "../../../Scene/SceneManager.h"
#include "../Enemy/EnemyBase.h"
#include "../Status/Status.h"
#include "../Magic/FireMagic/FireMagic.h"
#include "../Magic/IceMagic/IceMagic.h"
#include "../Magic/VoltMagic/VoltMagic.h"

#include <algorithm>
#include <vector>

namespace
{
	// ���@����̒��S�ł͂Ȃ��A������ɏo�����߂̍����B
	constexpr float MagicChantHeight = 1.5f;

	// �S�Ă̏񂪓�����]�ʒu���g�����߂̊�p�x�B
	float StaffOrbitBaseAngle = 0.0f;
}

void StaffBase::Init()
{
}

void StaffBase::Update()
{
	if (!IsMagicUnlocked())
	{
		return;
	}

	auto spTarget = m_wpTarget.lock();

	if (!spTarget)
	{
		return;
	}

	UpdateAroundTarget(spTarget);
	UpdateMagicAttack(spTarget);
}

void StaffBase::DrawLit()
{
	if (!IsMagicUnlocked())
	{
		return;
	}

	CharaBase::DrawLit();
}

void StaffBase::UpdateAroundTarget(const std::shared_ptr<KdGameObject>& spTarget)
{
	StaffLayoutInfo layoutInfo = GetLayoutInfo();
	if (layoutInfo.count <= 0 || layoutInfo.index < 0)
	{
		return;
	}

	// �S�Ă̏�œ�����p�x���g���A�擾�����ς���Ă��K�����Ԋu�ɕ��ׂ�B
	if (layoutInfo.index == 0)
	{
		StaffOrbitBaseAngle += m_rotateSpeed;
	}

	m_angle = StaffOrbitBaseAngle + GetLayoutOffset(layoutInfo.index, layoutInfo.count);

	float x = cos(m_angle) * m_radius;
	float z = sin(m_angle) * m_radius;

	m_pos = spTarget->GetPos() + Math::Vector3(x, m_height, z);

	m_mWorld = Math::Matrix::CreateTranslation(m_pos);
}

void StaffBase::UpdateMagicAttack(const std::shared_ptr<KdGameObject>& spPlayer)
{
	// ���@�^�C�v�����ݒ�̏�͍U�����Ȃ��B
	if (m_magicType == MagicType::None)
	{
		return;
	}

	// ���@�̃N�[���^�C�������炷�B
	m_magicCoolTime--;

	// �N�[���^�C�����c���Ă���Ȃ�A�܂������Ȃ��B
	if (m_magicCoolTime > 0.0f)
	{
		return;
	}

	std::shared_ptr<KdGameObject> spTargetEnemy = SearchEnemy(spPlayer);

	// �͈͓��ɓG�����Ȃ���Ό����Ȃ��B
	if (!spTargetEnemy)
	{
		return;
	}

	// �񂩂�G�֌��������������B
	Math::Vector3 shotDir = spTargetEnemy->GetPos() - GetPos();
	if (shotDir.LengthSquared() <= 0.0001f)
	{
		return;
	}
	shotDir.Normalize();

	// ���@������āA�G�̕����֔�΂��B
	Math::Vector3 chantPos = GetPos() + Math::Vector3(0.0f, MagicChantHeight, 0.0f);
	std::shared_ptr<KdGameObject> spStaff = shared_from_this();

	std::shared_ptr<Status> spStatus = m_wpStatus.lock();
	switch (m_magicType)
	{
	case MagicType::Fire:
	{
		const float explosionRadius = spStatus ? spStatus->GetFireExplosionRadius() : 3.0f;
		std::shared_ptr<FireMagic> magic = std::make_shared<FireMagic>();
		magic->Shot(chantPos, shotDir, m_magicType, m_magicDamage, m_magicSpeed, spStaff, spTargetEnemy, nullptr, explosionRadius);
		SceneManager::Instance().AddObject(magic);
		break;
	}
	case MagicType::Ice:
	{
		const int splitCount = spStatus ? spStatus->GetIceSplitCount() : 1;
		const int pierceCount = spStatus ? spStatus->GetIcePierceCount() : 1;
		std::shared_ptr<IceMagic> magic = std::make_shared<IceMagic>();
		magic->Shot(chantPos, shotDir, m_magicType, m_magicDamage, m_magicSpeed, spStaff, spTargetEnemy, nullptr, pierceCount, splitCount, false);
		SceneManager::Instance().AddObject(magic);
		break;
	}
	case MagicType::Volt:
	{
		const int chainCount = spStatus ? spStatus->GetVoltChainCount() : 1;
		std::shared_ptr<VoltMagic> magic = std::make_shared<VoltMagic>();
		magic->Shot(chantPos, shotDir, m_magicType, m_magicDamage, m_magicSpeed, spStaff, spTargetEnemy, nullptr, chainCount, false);
		SceneManager::Instance().AddObject(magic);
		break;
	}
	default:
		return;
	}

	// �񂲂Ƃɐݒ肳�ꂽ�N�[���^�C���֖߂��B
	m_magicCoolTime = m_magicCoolTimeMax;
}

std::shared_ptr<KdGameObject> StaffBase::SearchEnemy(const std::shared_ptr<KdGameObject>& spPlayer)
{
	std::shared_ptr<KdGameObject> spTargetEnemy = nullptr;
	float minDistanceSqr = m_searchRadius * m_searchRadius;

	for (auto& spObj : SceneManager::Instance().GetObjList())
	{
		// ���͓G��Bat�����Ȃ̂ŁAEnemyBase�ɕϊ��ł������̂��U���Ώۂɂ���B
		auto spEnemy = std::dynamic_pointer_cast<EnemyBase>(spObj);
		if (!spEnemy)
		{
			continue;
		}
		if (spEnemy->IsExpired())
		{
			continue;
		}

		Math::Vector3 toEnemy = spEnemy->GetPos() - spPlayer->GetPos();
		float distanceSqr = toEnemy.LengthSquared();

		if (distanceSqr < minDistanceSqr)
		{
			minDistanceSqr = distanceSqr;
			spTargetEnemy = spEnemy;
		}
	}

	return spTargetEnemy;
}

StaffBase::StaffLayoutInfo StaffBase::GetLayoutInfo() const
{
	// �V�[�����̉���ς݂̏񂾂����W�߁A���@�̎�ޏ��ɕ��ׂ�B
	std::vector<const StaffBase*> unlockedStaffs;

	for (auto& spObj : SceneManager::Instance().GetObjList())
	{
		auto spStaff = std::dynamic_pointer_cast<StaffBase>(spObj);
		if (!spStaff) { continue; }
		if (spStaff->m_magicType == MagicType::None) { continue; }
		if (!spStaff->IsMagicUnlocked()) { continue; }

		unlockedStaffs.push_back(spStaff.get());
	}

	std::sort(unlockedStaffs.begin(), unlockedStaffs.end(), [](const StaffBase* a, const StaffBase* b)
	{
		return a->GetMagicOrder() < b->GetMagicOrder();
	});

	StaffLayoutInfo info;
	info.count = static_cast<int>(unlockedStaffs.size());

	for (int i = 0; i < static_cast<int>(unlockedStaffs.size()); ++i)
	{
		if (unlockedStaffs[i] == this)
		{
			info.index = i;
			break;
		}
	}

	return info;
}

float StaffBase::GetLayoutOffset(int index, int count) const
{
	if (count <= 0) { return 0.0f; }

	// �����тɌ����ɂ����悤�ɁA�S�Ă̖{���őO���������ɓ��Ԋu�z�u����B
	const float startAngle = DirectX::XM_PIDIV2;
	const float angleStep = DirectX::XM_2PI / static_cast<float>(count);

	return startAngle + angleStep * static_cast<float>(index);
}

int StaffBase::GetMagicOrder() const
{
	switch (m_magicType)
	{
	case MagicType::Fire:
		return 0;
	case MagicType::Ice:
		return 1;
	case MagicType::Volt:
		return 2;
	default:
		return 99;
	}
}

bool StaffBase::IsMagicUnlocked() const
{
	std::shared_ptr<Status> spStatus = m_wpStatus.lock();
	if (!spStatus)
	{
		return true;
	}

	return spStatus->HasMagic(m_magicType);
}
