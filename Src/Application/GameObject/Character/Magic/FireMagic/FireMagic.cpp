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
	constexpr int FireSoundCount = 4;

	int g_lastFireShotSoundIndex = -1;
	int g_lastFireHitSoundIndex = -1;

	const char* FireShotSoundPathList[FireSoundCount] =
	{
		"Asset/Sounds/Magic/FireMagic/Shot/Fire_shot_01.wav",
		"Asset/Sounds/Magic/FireMagic/Shot/Fire_shot_02.wav",
		"Asset/Sounds/Magic/FireMagic/Shot/Fire_shot_03.wav",
		"Asset/Sounds/Magic/FireMagic/Shot/Fire_shot_04.wav"
	};

	const char* FireHitSoundPathList[FireSoundCount] =
	{
		"Asset/Sounds/Magic/FireMagic/Hit/Fire_hit_01.wav",
		"Asset/Sounds/Magic/FireMagic/Hit/Fire_hit_02.wav",
		"Asset/Sounds/Magic/FireMagic/Hit/Fire_hit_03.wav",
		"Asset/Sounds/Magic/FireMagic/Hit/Fire_hit_04.wav"
	};

	const char* GetRandomSoundPath(const char* const soundPathList[FireSoundCount], int& lastIndex)
	{
		int index = KdRandom::GetInt(0, FireSoundCount - 2);
		if (lastIndex >= 0 && index >= lastIndex)
		{
			++index;
		}

		lastIndex = index;
		return soundPathList[index];
	}
}

bool FireMagic::ConfigureShot(const MagicShotParams& params)
{
	m_explosionRadius = params.explosionRadius;
	return false; // 炎は通常どおり詠唱してから発射する。
}

void FireMagic::SetupMagic()
{
	MagicBase::SetupMagic();

	m_lifeFrames = 90.0f;
	m_radius = FireHitRadius;
	m_frameSpeed = FireFrameSpeed;
	m_spPoly->SetMaterial("Asset/Textures/Magic/Fire/Fire.png");
	m_spPoly->SetSplit(11, 1);
	m_spPoly->SetUVRect(m_flyFrameStart);
	m_spPoly->SetScale(FireScale);
	m_frameIndex = m_flyFrameStart;
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

	m_animFrame += m_frameSpeed;
	const int frameCount = m_flyFrameEnd - m_flyFrameStart + 1;
	const int frameIndex = m_flyFrameStart + (static_cast<int>(m_animFrame) % frameCount);

	if (frameIndex != m_frameIndex)
	{
		m_frameIndex = frameIndex;
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

	m_animFrame += m_frameSpeed;
	const int frameIndex = m_hitFrameStart + static_cast<int>(m_animFrame);

	if (frameIndex > m_hitFrameEnd)
	{
		m_isExpired = true;
		return;
	}

	if (frameIndex != m_frameIndex)
	{
		m_frameIndex = frameIndex;
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
	m_animFrame = 0.0f;
	m_frameIndex = -1;

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
	return GetRandomSoundPath(FireShotSoundPathList, g_lastFireShotSoundIndex);
}

const char* FireMagic::GetHitSoundPath() const
{
	return GetRandomSoundPath(FireHitSoundPathList, g_lastFireHitSoundIndex);
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
	const float blastRadiusSq = m_explosionRadius * m_explosionRadius;

	for (const std::weak_ptr<EnemyBase>& wpEnemy : SceneManager::Instance().GetActiveEnemies())
	{
		std::shared_ptr<EnemyBase> spEnemy = wpEnemy.lock();
		if (!spEnemy) { continue; }
		if (spEnemy == hitEnemy) { continue; }
		if (spEnemy->IsExpired()) { continue; }

		const Math::Vector3 toEnemy = spEnemy->GetPos() - explosionCenter;
		if (toEnemy.LengthSquared() <= blastRadiusSq)
		{
			// 範囲内の敵にも同じ炎ダメージを与える.
			spEnemy->OnHit(m_damage);
			AddHitObject(spEnemy);
		}
	}
}
