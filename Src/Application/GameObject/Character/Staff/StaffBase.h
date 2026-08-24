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

	// �񂲂Ƃ̖��@���\��ݒ肷��B
	void SetMagicParam(MagicType type, float damage, float speed, float coolTime)
	{
		m_magicType = type;
		m_magicDamage = damage;
		m_magicSpeed = speed;
		m_magicCoolTimeMax = coolTime;
		m_magicCoolTime = 0.0f;
	}

	std::weak_ptr<KdGameObject> m_wpTarget;
	std::weak_ptr<Status> m_wpStatus;

private:
	// �\�����̏񂾂��ŁA���������Ԗڂɕ��Ԃ������B
	struct StaffLayoutInfo
	{
		int count = 0;
		int index = -1;
	};

	// �v���C���[�̎������鏈���B
	void UpdateAroundTarget(const std::shared_ptr<KdGameObject>& spTarget);

	// ���G���Ė��@���������B
	void UpdateMagicAttack(const std::shared_ptr<KdGameObject>& spPlayer);

	// �v���C���[�̋߂��ɂ����ԋ߂��G��T���B
	std::shared_ptr<KdGameObject> SearchEnemy(const std::shared_ptr<KdGameObject>& spPlayer);

	bool IsMagicUnlocked() const;
	StaffLayoutInfo GetLayoutInfo() const;
	float GetLayoutOffset(int index, int count) const;
	int GetMagicOrder() const;

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


