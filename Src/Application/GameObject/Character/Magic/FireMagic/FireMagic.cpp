#include "FireMagic.h"

#include "../../../../Scene/SceneManager.h"
#include "../../Enemy/EnemyBase.h"

#include <algorithm>

namespace
{
	constexpr float FireScale = 4.0f;
	constexpr float FireFrameSpeed = 0.25f;
	constexpr float FireBaseExplosionRadius = 3.0f;
	constexpr float FireHitRadius = FireScale * 0.5f;
}

void FireMagic::Shot(
	const Math::Vector3& startPos,
	const Math::Vector3& dir,
	MagicType type,
	float damage,
	float speed,
	const std::shared_ptr<KdGameObject>& chantTarget,
	const std::shared_ptr<KdGameObject>& flyTarget,
	const std::shared_ptr<KdGameObject>& ignoreTarget,
	float explosionRadius)
{
	m_explosionRadius = explosionRadius;

	// 共通の発射初期化はBaseへ任せ、炎専用の爆発範囲だけこのクラスで持つ.
	MagicBase::Shot(startPos, dir, type, damage, speed, chantTarget, flyTarget, ignoreTarget);
}

void FireMagic::SetupMagic()
{
	MagicBase::SetupMagic();

	m_lifeTime = 90.0f;
	m_radius = FireHitRadius;
	m_frameSpeed = FireFrameSpeed;
	m_spPoly->SetMaterial("Asset/Textures/Magic/Fire/Fire.png");
	m_spPoly->SetSplit(11, 1);
	m_spPoly->SetUVRect(m_flyFrameStart);
	m_spPoly->SetScale(FireScale);
	m_nowFrame = m_flyFrameStart;
}

void FireMagic::UpdateChantMagic()
{
	if (m_spPoly)
	{
		m_spPoly->SetUVRect(m_flyFrameStart);
	}
}

void FireMagic::UpdateFlyMagic()
{
	if (!m_spPoly) { return; }

	m_frame += m_frameSpeed;
	const int frameCount = m_flyFrameEnd - m_flyFrameStart + 1;
	const int frameIndex = m_flyFrameStart + (static_cast<int>(m_frame) % frameCount);

	if (frameIndex != m_nowFrame)
	{
		m_nowFrame = frameIndex;
		m_spPoly->SetUVRect(frameIndex);
	}
}

void FireMagic::UpdateHitMagic()
{
	if (!m_spPoly)
	{
		m_isExpired = true;
		return;
	}

	m_frame += m_frameSpeed;
	const int frameIndex = m_hitFrameStart + static_cast<int>(m_frame);

	if (frameIndex > m_hitFrameEnd)
	{
		m_isExpired = true;
		return;
	}

	if (frameIndex != m_nowFrame)
	{
		m_nowFrame = frameIndex;
		m_spPoly->SetUVRect(frameIndex);
	}
}

void FireMagic::OnAfterDamage(const std::shared_ptr<EnemyBase>& hitEnemy)
{
	// 直撃した敵へのダメージ後に、範囲ダメージを追加する.
	ApplyExplosion(hitEnemy);
}

bool FireMagic::StartHitAnimation()
{
	m_frame = 0.0f;
	m_nowFrame = -1;

	if (m_spPoly)
	{
		// 爆発範囲が広がった分だけ、命中演出の見た目も大きくする.
		const float addScale = std::max(m_explosionRadius - FireBaseExplosionRadius, 0.0f);
		m_spPoly->SetScale(FireScale + addScale);
		m_spPoly->SetUVRect(m_hitFrameStart);
	}

	return true;
}

const char* FireMagic::GetShotSoundPath() const
{
	return "Asset/Sounds/Magic/FireMagic/shot.wav";
}

const char* FireMagic::GetHitSoundPath() const
{
	return "Asset/Sounds/Magic/FireMagic/explosion.wav";
}

float FireMagic::GetDirectionAngleOffset() const
{
	return DirectX::XM_PI;
}

Math::Vector3 FireMagic::GetEmissiveColor() const
{
	return { 0.45f, 0.18f, 0.03f };
}

void FireMagic::ApplyExplosion(const std::shared_ptr<EnemyBase>& hitEnemy)
{
	if (!hitEnemy) { return; }
	if (m_explosionRadius <= 0.0f) { return; }

	const Math::Vector3 explosionCenter = hitEnemy->GetPos();
	const float explosionRadiusSqr = m_explosionRadius * m_explosionRadius;

	for (const std::shared_ptr<KdGameObject>& spObj : SceneManager::Instance().GetObjList())
	{
		if (!spObj) { continue; }

		std::shared_ptr<EnemyBase> spEnemy = std::dynamic_pointer_cast<EnemyBase>(spObj);
		if (!spEnemy) { continue; }
		if (spEnemy == hitEnemy) { continue; }
		if (spEnemy->IsExpired()) { continue; }

		const Math::Vector3 toEnemy = spEnemy->GetPos() - explosionCenter;
		if (toEnemy.LengthSquared() <= explosionRadiusSqr)
		{
			// 範囲内の敵にも同じ炎ダメージを与える.
			spEnemy->OnHit(m_damage);
			AddHitObject(spEnemy);
		}
	}
}
