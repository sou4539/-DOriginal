#include "GrudgeBat.h"

namespace
{
	// 通常のHP30・接触ダメージ5・経験値20の2倍。調整はここで行う。
	constexpr float InitialHp = 60.0f;
	constexpr float InitialDamage = 10.0f;
	constexpr float InitialExp = 40.0f;
}

GrudgeBat::GrudgeBat(const std::shared_ptr<GrudgeBattleState>& battle) : m_battle(battle)
{
	RefreshStrength();
}

void GrudgeBat::RefreshStrength()
{
	if (!m_battle || !CanBeTargeted() || m_appliedDefeatedCount == m_battle->DefeatedCount) { return; }
	const float multiplier = m_battle->GetMultiplier();
	SetCombatStats(InitialHp * multiplier, InitialExp * multiplier);
	m_appliedDefeatedCount = m_battle->DefeatedCount;
}

void GrudgeBat::Update()
{
	RefreshStrength();
	// 移動速度・死亡後の停止・ディゾルブは通常のコウモリと共通。
	Bat::Update();
}

void GrudgeBat::OnHit(float damage)
{
	// 同フレームに連鎖や爆発で複数体が倒されても、最新の倍率で判定する。
	RefreshStrength();
	Bat::OnHit(damage);
}

float GrudgeBat::GetContactDamage() const
{
	return InitialDamage * (m_battle ? m_battle->GetMultiplier() : 1.0f);
}

void GrudgeBat::OnDefeated()
{
	// Batの死亡処理から一度だけ呼ばれ、残り全員の強化倍率を更新する。
	if (m_battle) { ++m_battle->DefeatedCount; }
}

void GrudgeBat::DrawLit()
{
	if (!m_spModel) { return; }
	KdModelVisualEffects effects;
	// 怨念は青白い輪郭。被弾時は赤い輪郭に切り替える。
	effects.RimLight = true;
	effects.RimColor = IsHitFlashing() ? Math::Vector3(3.0f, 0.1f, 0.1f) : Math::Vector3(0.3f, 0.7f, 1.0f);
	effects.RimPower = 2.0f;
	KdModelInstanceBatcher::Instance().SubmitLit(m_spModel, m_mWorld,
		Math::Color(0.6f, 0.8f, 1.0f, 1.0f), Math::Vector3::Zero, GetDissolveValue(), effects);
}
