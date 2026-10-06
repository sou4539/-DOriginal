#pragma once

#include "../CharaBase.h"
#include "../Magic/MagicBase.h"

class Status;

class StaffBase :public CharaBase
{
public:
	StaffBase() { Init(); };
	~StaffBase() {};

	virtual void Init() override ;
	virtual void Update() override ;
	void DrawLit() override;

	void SetTarget(const std::weak_ptr<KdGameObject>& target)
	{ 
		m_wpTarget = target; 
	}

	void SetStatus(const std::shared_ptr<Status>& status)
	{
		m_wpStatus = status;
	}

protected:

	// 杖ごとの魔法性能を設定する。
	void SetMagicParam(MagicType type, float damage, float speed, float cooldown)
	{
		m_magicType = type;
		m_damage = damage;
		m_shotSpeed = speed;
		m_cooldownMax = cooldown;
		m_cooldown = 0.0f;
	}

	std::weak_ptr<KdGameObject> m_wpTarget;
	std::weak_ptr<Status> m_wpStatus;

private:
	// 表示中の杖が全部で何本あり、自分が何番目かを持つ。
	struct StaffLayoutInfo
	{
		int count = 0;
		int index = -1;
	};

	// プレイヤーの周りを回る処理。
	void UpdateAroundTarget(const std::shared_ptr<KdGameObject>& spTarget);

	// 敵を探して魔法を撃つ処理。
	void UpdateMagicAttack(const std::shared_ptr<KdGameObject>& spPlayer);

	// プレイヤーの近くにいる一番近い敵を探す。
	std::shared_ptr<KdGameObject> SearchEnemy(const std::shared_ptr<KdGameObject>& spPlayer);

	bool IsMagicUnlocked() const;
	StaffLayoutInfo GetLayoutInfo() const;
	float GetLayoutOffset(int index, int count) const;
	int GetMagicOrder() const;

	float m_orbitAngle = 0.0f;
	float m_orbitRadius = 1.5f;
	float m_orbitHeight = 2.0f;
	float m_orbitSpeed = 0.03f;

	MagicType m_magicType = MagicType::None;
	float m_damage = 0.0f;
	float m_shotSpeed = 0.0f;
	float m_cooldown = 0.0f;
	float m_cooldownMax = 60.0f;
	float m_searchRadius = 14.0f;

};


