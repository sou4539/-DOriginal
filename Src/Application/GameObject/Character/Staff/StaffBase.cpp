#include "StaffBase.h"

#include "../../../Scene/SceneManager.h"
#include "../Bat/Bat.h"

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
	std::shared_ptr<MagicBase> magic = std::make_shared<MagicBase>();
	magic->Shot(GetPos(), shotDir, m_magicType, m_magicDamage, m_magicSpeed);
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


