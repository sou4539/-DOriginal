#pragma once

#include "../CharaBase.h"

class Status;

class Bat : public CharaBase
{
public:
	Bat() { Init(); }
	~Bat() override {}

	void Init() override;
	void Update() override;
	void DrawLit() override;

	// 魔法などが当たった時に呼ぶ。
	// HPが0以下になったら、シーンから削除されるようにする。
	void OnHit() override;

	// 魔法などからダメージ量を指定して呼ぶ。
	// 杖ごとの攻撃力差や、今後のレベル補正を反映したい時はこちらを使う。
	void OnHit(float damage);

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
	// 移動を始めた後はUpdate()内で移動方向に合わせて上書きされる。
	void SetAngle(float angle) { m_angle = angle; }

	// BatGroup側から、出現範囲外に出たコウモリを消す時に使う。
	// HPを減らして倒した時とは違い、経験値は入れずにシーンから削除する。
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
	// ここに入った瞬間、追跡状態へ切り替える。
	float m_searchRadius = 8.0f;

	// 一度プレイヤーを見つけた後の追跡継続範囲。
	// 発見前より広くして、少し離れただけでは追跡をやめないようにする。
	float m_chaseRadius = 16.0f;

	// trueなら、現在プレイヤーを追跡中。
	// falseなら、発見範囲に入るまでは初期位置へ戻る。
	bool m_isChasing = false;

	float m_damageRadius = 0.7f;
	float m_moveSpeed = 0.08f;
	float m_hp = 30.0f;
	float m_exp = 20.0f;

	// 被弾した瞬間だけ赤く表示するための残りフレーム。
	int m_hitFlashFrame = 0;
};












