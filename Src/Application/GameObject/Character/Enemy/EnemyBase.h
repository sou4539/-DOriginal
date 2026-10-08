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

	// 接触ダメージ。通常の敵は従来の5、怨念は強化倍率に応じて変更する。
	virtual float GetContactDamage() const { return 5.0f; }

	// スポナーなどから敵を消す時に使う。
	void Expire() { m_isExpired = true; }

	// 敵の生存確認
	virtual bool CanBeTargeted() const
	{
		return !m_isExpired;
	}
};
