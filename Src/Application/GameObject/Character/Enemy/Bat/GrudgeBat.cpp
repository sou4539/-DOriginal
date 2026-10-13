#include "GrudgeBat.h"

#include "../../../../Scene/SceneManager.h"

namespace
{
	// 通常のHP30・接触ダメージ5・経験値20の2倍。調整はここで行う。
	constexpr float InitialHp = GameBalance::GrudgeInitialHp;
	constexpr float InitialDamage = GameBalance::GrudgeInitialDamage;
	constexpr float InitialExp = GameBalance::GrudgeInitialExp;
}

GrudgeBat::GrudgeBat(const std::shared_ptr<GrudgeBattleState>& battle) : m_battle(battle)
{
	m_cbjType = GhostObj;
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
	m_cbjType = GhostObj;
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
	if (m_battle)
	{
		++m_battle->DefeatedCount;
		m_battle->StrengthBonus += GameBalance::GrudgeStrengthPerKill;
	}
}

void GrudgeBat::DrawLit()
{
	if (!m_spModel) { return; }
	KdModelVisualEffects effects;
	effects.RimLight = IsHitFlashing();
	effects.RimColor = Math::Vector3(3.0f, 0.1f, 0.1f);
	effects.RimPower = 2.0f;
	effects.OpticalCamo = true;
	effects.ScreenTexture = SceneManager::Instance().GetRenderTargetTexture();
	KdModelInstanceBatcher::Instance().SubmitLit(m_spModel, m_mWorld,
		kWhiteColor, Math::Vector3::Zero, GetDissolveValue(), effects);
}
