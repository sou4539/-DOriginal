#pragma once

#include "../CharaBase.h"

class EnemyBase : public CharaBase
{
public:
	EnemyBase() {}
	~EnemyBase() override {}

	// 魔法などからダメージ量を指定して呼ぶ。
	virtual void OnHit(float) {}

	// 敵として扱えるかを外部から確認する。
	bool IsEnemy() const { return true; }

	// スポナーなどから敵を消す時に使う。
	void Expire() { m_isExpired = true; }
};