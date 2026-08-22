#include "StaffBase.h"

#include "../../../Scene/SceneManager.h"
#include "../Bat/Bat.h"
#include "../Status/Status.h"

namespace
{
	// 魔法を杖の中心ではなく、少し上に出すための高さ。
	// 詠唱中はこの高さを保ったまま杖に追従する。
	constexpr float MagicChantHeight = 1.5f;
}

void StaffBase::Init()
{
}

void StaffBase::Update()
{
	auto spTarget = m_wpTarget.lock();

	if (!spTarget)
	{
		return;
	}

	UpdateAroundTarget(spTarget);
	UpdateMagicAttack(spTarget);
}

void StaffBase::UpdateAroundTarget(const std::shared_ptr<KdGameObject>& spTarget)
{
	m_angle += m_rotateSpeed;

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
	// 詠唱中は杖の上に表示したいので、開始位置は杖の少し上にする。
	// shared_from_this()で杖自身を渡すことで、魔法側は詠唱中だけ杖に追従できる。
	// 敵も渡しておき、発射する瞬間に現在の敵位置へY方向込みで向き直す。
	Math::Vector3 chantPos = GetPos() + Math::Vector3(0.0f, MagicChantHeight, 0.0f);
	std::shared_ptr<KdGameObject> spStaff = shared_from_this();

	std::shared_ptr<Status> spStatus = m_wpStatus.lock();
	const float fireExplosionRadius = spStatus ? spStatus->GetFireExplosionRadius() : 3.0f;
	const int iceSplitCount = (m_magicType == MagicType::Ice && spStatus) ? spStatus->GetIceSplitCount() : 1;
	const int icePierceCount = (m_magicType == MagicType::Ice && spStatus) ? spStatus->GetIcePierceCount() : 1;
	const int voltChainCount = (m_magicType == MagicType::Volt && spStatus) ? spStatus->GetVoltChainCount() : ((m_magicType == MagicType::Volt) ? 1 : 0);

	// ここで生成する魔法は常に1発だけにする。
	// 氷の強化は「初弾を増やす」のではなく、命中後の派生弾としてMagicBase側で処理する。
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
		// 今は敵がBatだけなので、Batに変換できたものだけを攻撃対象にする。
		// 後でEnemyBaseを作ったら、ここをEnemyBase判定に変更する。
		auto spBat = std::dynamic_pointer_cast<Bat>(spObj);
		if (!spBat)
		{
			continue;
		}
		if (spBat->IsExpired())
		{
			continue;
		}

		Math::Vector3 toEnemy = spBat->GetPos() - spPlayer->GetPos();
		float distanceSqr = toEnemy.LengthSquared();

		if (distanceSqr < minDistanceSqr)
		{
			minDistanceSqr = distanceSqr;
			spTargetEnemy = spBat;
		}
	}

	return spTargetEnemy;
}














