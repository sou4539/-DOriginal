#pragma once

#include "../EnemyBase.h"

class Status;

class Bat : public EnemyBase
{
public:
	Bat() { Init(); }
	~Bat() override {}

	void Init() override;
	void Update() override;
	void DrawLit() override;
	void GenerateDepthMapFromLight() override;
	void DrawEffect() override;

	// 魔法などが当たった時に呼ぶ。
	void OnHit() override;

	// 魔法などからダメージ量を指定して呼ぶ。
	void OnHit(float damage) override;

	void SetTarget(const std::shared_ptr<KdGameObject>& target)
	{
		m_wpTarget = target;
	}

	void SetStatus(const std::shared_ptr<Status>& status)
	{
		m_wpStatus = status;
	}

	void SetStartPos(const Math::Vector3& pos)
	{
		m_pos = pos;
		m_startPos = pos;
		m_isChasing = false;
		SetPos(pos);
	}

	// 召喚直後の見た目の向きを設定する。
	void SetAngle(float angle) { m_angle = angle; }

	// EnemySpawner側から、出現範囲外に出たコウモリを消す時に使う。
	void Expire() { m_isExpired = true; }

private:
	/*
		Bat.gltfに入っているアニメーションを再生・更新するためのクラス。
		今回は羽ばたき用の "flap_loop" を再生する。
	*/
	KdAnimator m_animator;

	std::weak_ptr<KdGameObject> m_wpTarget;
	std::weak_ptr<Status> m_wpStatus;

	Math::Vector3 m_startPos = Math::Vector3::Zero;

	float m_angle = 0.0f;

	// プレイヤーをまだ見つけていない時の発見範囲。
	float m_searchRadius = 12.0f;

	// 一度プレイヤーを見つけた後の追跡継続範囲。
	float m_chaseRadius = 24.0f;

	// trueなら、現在プレイヤーを追跡中。
	bool m_isChasing = false;
	int m_hitFlashFrame = 0;
	int m_animUpdateFrame = 0;

	float m_damageRadius = 0.7f;
	float m_moveSpeed = 0.11f;
	float m_hp = 30.0f;
	float m_exp = 20.0f;
};












