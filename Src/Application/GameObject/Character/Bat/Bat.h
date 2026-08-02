#pragma once

#include "../CharaBase.h"

class Bat : public CharaBase
{
public:
	Bat() { Init(); }
	~Bat() override {}

	void Init() override;
	void Update() override;

	void SetTarget(const std::shared_ptr<KdGameObject>& target)
	{
		m_wpTarget = target;
	}

	void SetStartPos(const Math::Vector3& pos)
	{
		m_pos = pos;
		m_startPos = pos;
		SetPos(pos);
	}

private:
	/*
		Bat.gltfに入っているアニメーションを再生・更新するためのクラス。
		今回は羽ばたき用の "flap_loop" を再生する。
	*/
	KdAnimator m_animator;

	std::weak_ptr<KdGameObject> m_wpTarget;

	Math::Vector3 m_startPos = Math::Vector3::Zero;

	float m_angle = 0.0f;
	float m_searchRadius = 8.0f;
	float m_damageRadius = 0.7f;
	float m_moveSpeed = 0.08f;
};
