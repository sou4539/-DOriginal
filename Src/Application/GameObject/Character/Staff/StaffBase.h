#pragma once

#include "../CharaBase.h"
#include "Magic/MagicBase.h"

class StaffBase :public CharaBase
{
public:
	StaffBase() { Init(); };
	~StaffBase() {};

	virtual void Init() override ;
	virtual void Update() override ;

	void SetTarget(const std::weak_ptr<KdGameObject>& target)
	{ 
		m_wpTarget = target; 
	}

	void SetAngle(float angle)
	{
		m_angle = angle;
	}

protected:

	// 杖ごとの魔法性能を設定する。
	// 各杖のInitで呼ぶことで、攻撃処理はStaffBase側で共通化できる。
	void SetMagicParam(MagicType type, float damage, float speed, float coolTime)
	{
		m_magicType = type;
		m_magicDamage = damage;
		m_magicSpeed = speed;
		m_magicCoolTimeMax = coolTime;
		m_magicCoolTime = 0.0f;
	}

	std::weak_ptr<KdGameObject> m_wpTarget;

private:

	// プレイヤーの周りを回る処理。
	void UpdateAroundTarget(const std::shared_ptr<KdGameObject>& spTarget);

	// 索敵して魔法を撃つ処理。
	void UpdateMagicAttack(const std::shared_ptr<KdGameObject>& spPlayer);

	// プレイヤーの近くにいる一番近い敵を探す。
	std::shared_ptr<KdGameObject> SearchEnemy(const std::shared_ptr<KdGameObject>& spPlayer);

	float m_angle = 0.0f;
	float m_radius = 1.5f;
	float m_height = 2.0f;
	float m_rotateSpeed = 0.03f;

	MagicType m_magicType = MagicType::None;
	float m_magicDamage = 0.0f;
	float m_magicSpeed = 0.0f;
	float m_magicCoolTime = 0.0f;
	float m_magicCoolTimeMax = 60.0f;
	float m_searchRadius = 8.0f;

};
