#include "StaffBase.h"

#include "../../../Scene/SceneManager.h"
#include "../Enemy/EnemyBase.h"
#include "../Status/Status.h"
#include "../Magic/MagicBase.h"
#include "../Magic/FireMagic/FireMagic.h"
#include "../Magic/IceMagic/IceMagic.h"
#include "../Magic/VoltMagic/VoltMagic.h"
#include <algorithm>
#include <vector>

namespace
{
	// 魔法を杖の中心ではなく、少し上から出すための高さ。
	constexpr float MagicChantHeight = 1.5f;

	// すべての杖が同じ回転基準を使うための基準角度。
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
	StaffLayoutInfo layout = GetLayoutInfo();
	if (layout.count <= 0 || layout.index < 0)
	{
		return;
	}

	// すべての杖で同じ基準角度を使い、取得済みの杖を等間隔に並べる。
	if (layout.index == 0)
	{
		StaffOrbitBaseAngle += m_orbitSpeed;
	}

	m_orbitAngle = StaffOrbitBaseAngle + GetLayoutOffset(layout.index, layout.count);

	float x = cos(m_orbitAngle) * m_orbitRadius;
	float z = sin(m_orbitAngle) * m_orbitRadius;

	m_pos = spTarget->GetPos() + Math::Vector3(x, m_orbitHeight, z);

	m_mWorld = Math::Matrix::CreateTranslation(m_pos);
}

void StaffBase::UpdateMagicAttack(const std::shared_ptr<KdGameObject>& spPlayer)
{
	// 魔法タイプが未設定なら攻撃しない。
	if (m_magicType == MagicType::None)
	{
		return;
	}

	// 魔法のクールタイムを減らす。
	m_cooldown--;

	// クールタイムが残っているなら、まだ撃たない。
	if (m_cooldown > 0.0f)
	{
		return;
	}

	std::shared_ptr<KdGameObject> spTargetEnemy = SearchEnemy(spPlayer);

	// 範囲内に敵がいなければ撃たない。
	if (!spTargetEnemy)
	{
		return;
	}

	// 杖から敵へ向かう発射方向を作る。
	Math::Vector3 shotDir = spTargetEnemy->GetPos() - GetPos();
	if (shotDir.LengthSquared() <= 0.0001f)
	{
		return;
	}
	shotDir.Normalize();

	// 魔法を生成して、敵の方向へ飛ばす。
	Math::Vector3 chantPos = GetPos() + Math::Vector3(0.0f, MagicChantHeight, 0.0f);
	std::shared_ptr<KdGameObject> spStaff = shared_from_this();

	std::shared_ptr<Status> spStatus = m_wpStatus.lock();

	// 現在の攻撃力を確認
	const float attack = spStatus ? spStatus->GetPlayerAttack() : 10.0f;
	// 魔法自体の倍率と掛け合わせる
	const float magicDamage = m_damage * (attack / 10.0f);

	// 全魔法で共通の発射条件をまとめる。
	MagicShotParams params;
	params.startPos = chantPos;
	params.dir = shotDir;
	params.damage = magicDamage;
	params.speed = m_shotSpeed;
	params.chantTarget = spStaff;
	params.flyTarget = spTargetEnemy;

	// 変数の型はBaseでも、実体は各魔法にする。
	// make_shared<MagicBase>()では氷・雷・炎の専用処理は呼ばれない。
	std::shared_ptr<MagicBase> magic;
	switch (m_magicType)
	{
	case MagicType::Fire:
		magic = std::make_shared<FireMagic>();
		params.explosionRadius = spStatus ? spStatus->GetFireExplosionRadius() : 3.0f;
		break;
	case MagicType::Ice:
		magic = std::make_shared<IceMagic>();
		params.splitCount = spStatus ? spStatus->GetIceSplitCount() : 0;
		params.pierceCount = spStatus ? spStatus->GetIcePierceCount() : 1;
		break;
	case MagicType::Volt:
		magic = std::make_shared<VoltMagic>();
		params.chainCount = spStatus ? spStatus->GetVoltChainCount() : 1;
		break;
	default:
		return;
	}

	// 入口は1つ。Shot内部のvirtual関数が実体に応じた専用処理を呼ぶ。
	magic->Shot(params);
	SceneManager::Instance().AddObject(magic);

	// 杖ごとに設定されたクールタイムへ戻す。
	m_cooldown = m_cooldownMax;
}

std::shared_ptr<KdGameObject> StaffBase::SearchEnemy(const std::shared_ptr<KdGameObject>& spPlayer)
{
	std::shared_ptr<KdGameObject> spTargetEnemy = nullptr;
	float minDistSq = m_searchRadius * m_searchRadius;

	for (const std::weak_ptr<EnemyBase>& wpEnemy : SceneManager::Instance().GetActiveEnemies())
	{
		auto spEnemy = wpEnemy.lock();
		if (!spEnemy)
		{
			continue;
		}
		if (!spEnemy->CanBeTargeted())
		{ 
			continue; 
		}

		Math::Vector3 toEnemy = spEnemy->GetPos() - spPlayer->GetPos();
		float distSq = toEnemy.LengthSquared();

		if (distSq < minDistSq)
		{
			minDistSq = distSq;
			spTargetEnemy = spEnemy;
		}
	}

	return spTargetEnemy;
}

StaffBase::StaffLayoutInfo StaffBase::GetLayoutInfo() const
{
	// シーン内の取得済みの杖だけを集め、魔法の種類順に並べる。
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

	// 横並びに見えないよう、すべての本数で前方向を基準に等間隔配置する。
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
