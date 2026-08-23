#include "StaffBase.h"

#include "../../../Scene/SceneManager.h"
#include "../Enemy/EnemyBase.h"
#include "../Status/Status.h"

#include <algorithm>
#include <vector>

namespace
{
	// 魔法を杖の中心ではなく、少し上に出すための高さ。
	constexpr float MagicChantHeight = 1.5f;

	// 全ての杖が同じ回転位置を使うための基準角度。
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

	// 全ての杖で同じ基準角度を使い、取得数が変わっても必ず等間隔に並べる。
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
	// 魔法タイプが未設定の杖は攻撃しない。
	if (m_magicType == MagicType::None)
	{
		return;
	}

	// 魔法のクールタイムを減らす。
	m_magicCoolTime--;

	// クールタイムが残っているなら、まだ撃たない。
	if (m_magicCoolTime > 0.0f)
	{
		return;
	}

	std::shared_ptr<KdGameObject> spTargetEnemy = SearchEnemy(spPlayer);

	// 範囲内に敵がいなければ撃たない。
	if (!spTargetEnemy)
	{
		return;
	}

	// 杖から敵へ向かう方向を作る。
	Math::Vector3 shotDir = spTargetEnemy->GetPos() - GetPos();
	if (shotDir.LengthSquared() <= 0.0001f)
	{
		return;
	}
	shotDir.Normalize();

	// 魔法を作って、敵の方向へ飛ばす。
	Math::Vector3 chantPos = GetPos() + Math::Vector3(0.0f, MagicChantHeight, 0.0f);
	std::shared_ptr<KdGameObject> spStaff = shared_from_this();

	std::shared_ptr<Status> spStatus = m_wpStatus.lock();
	const float fireExplosionRadius = spStatus ? spStatus->GetFireExplosionRadius() : 3.0f;
	const int iceSplitCount = (m_magicType == MagicType::Ice && spStatus) ? spStatus->GetIceSplitCount() : 1;
	const int icePierceCount = (m_magicType == MagicType::Ice && spStatus) ? spStatus->GetIcePierceCount() : 1;
	const int voltChainCount = (m_magicType == MagicType::Volt && spStatus) ? spStatus->GetVoltChainCount() : ((m_magicType == MagicType::Volt) ? 1 : 0);

	// ここで生成する魔法は常に1発だけにする。
	std::shared_ptr<MagicBase> magic = std::make_shared<MagicBase>();
	magic->Shot
	(
		chantPos,
		shotDir,
		m_magicType,
		m_magicDamage,
		m_magicSpeed,
		spStaff,
		spTargetEnemy,
		voltChainCount,
		nullptr,
		false,
		fireExplosionRadius,
		icePierceCount,
		iceSplitCount,
		false
	);
	SceneManager::Instance().AddObject(magic);

	// 杖ごとに設定されたクールタイムへ戻す。
	m_magicCoolTime = m_magicCoolTimeMax;
}

std::shared_ptr<KdGameObject> StaffBase::SearchEnemy(const std::shared_ptr<KdGameObject>& spPlayer)
{
	std::shared_ptr<KdGameObject> spTargetEnemy = nullptr;
	float minDistanceSqr = m_searchRadius * m_searchRadius;

	for (auto& spObj : SceneManager::Instance().GetObjList())
	{
		// 今は敵がBatだけなので、EnemyBaseに変換できたものを攻撃対象にする。
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
	// シーン内の解放済みの杖だけを集め、魔法の種類順に並べる。
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

	// 横並びに見えにくいように、全ての本数で前後方向を基準に等間隔配置する。
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
