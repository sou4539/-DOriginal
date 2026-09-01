#include "IceMagic.h"

#include "../../../../Scene/SceneManager.h"
#include "../../Enemy/EnemyBase.h"

namespace
{
	constexpr float IceScale = 4.0f;
	constexpr float IceFrameSpeed = 0.12f;
	constexpr float IceHitRadius = IceScale * 0.5f;
	constexpr int IceMinSplitCount = 2;
	constexpr int IceMaxSplitCount = 8;
	constexpr float IceSplitTotalSpreadAngle = DirectX::XMConvertToRadians(90.0f);
	constexpr float IceSplitForwardOffset = 1.5f;
	constexpr float IceSplitSideOffset = 1.6f;
	constexpr int IceSoundCount = 4;

	int g_lastIceShotSoundIndex = -1;
	int g_lastIceHitSoundIndex = -1;

	const char* IceShotSoundPathList[IceSoundCount] =
	{
		"Asset/Sounds/Magic/IceMagic/Shot/Ice_shot_01.wav",
		"Asset/Sounds/Magic/IceMagic/Shot/Ice_shot_02.wav",
		"Asset/Sounds/Magic/IceMagic/Shot/Ice_shot_03.wav",
		"Asset/Sounds/Magic/IceMagic/Shot/Ice_shot_04.wav"
	};

	const char* IceHitSoundPathList[IceSoundCount] =
	{
		"Asset/Sounds/Magic/IceMagic/Hit/Ice_hit_01.wav",
		"Asset/Sounds/Magic/IceMagic/Hit/Ice_hit_02.wav",
		"Asset/Sounds/Magic/IceMagic/Hit/Ice_hit_03.wav",
		"Asset/Sounds/Magic/IceMagic/Hit/Ice_hit_04.wav"
	};

	const char* GetRandomSoundPath(const char* const soundPathList[IceSoundCount], int& lastIndex)
	{
		int index = KdRandom::GetInt(0, IceSoundCount - 2);
		if (lastIndex >= 0 && index >= lastIndex)
		{
			++index;
		}

		lastIndex = index;
		return soundPathList[index];
	}
}

void IceMagic::Shot(
	const Math::Vector3& startPos,						// 発射位置
	const Math::Vector3& dir,							// 発射方向
	MagicType type,										// 魔法の種類
	float damage,										// ダメージ量
	float speed,										// 速度
	const std::shared_ptr<KdGameObject>& chantTarget,	// 詠唱対象
	const std::shared_ptr<KdGameObject>& flyTarget,		// 飛翔対象
	const std::shared_ptr<KdGameObject>& ignoreTarget,	// 無視対象
	int pierceCount,									// 貫通数
	int splitCount,										// 分散数
	bool isSplitShot)									// 派生弾かどうか
{
	m_pierceCount = pierceCount;
	m_isSplitShot = isSplitShot;
	m_splitCount = m_isSplitShot ? 0 : std::clamp(splitCount, IceMinSplitCount, IceMaxSplitCount);
	m_hasCreatedSplit = false;

	// 共通の発射初期化はBaseへ任せ、氷専用の値だけこのクラスで持つ.
	MagicBase::Shot(startPos, dir, type, damage, speed, chantTarget, flyTarget, ignoreTarget);

	if (m_isSplitShot)
	{
		// 派生弾は詠唱せず、生成された瞬間から飛ばす.
		StartFly();
		UpdateWorldMatrix();
	}
}

void IceMagic::SetupMagic()
{
	MagicBase::SetupMagic();

	m_lifeTime = 120.0f;
	m_radius = IceHitRadius;
	m_frameSpeed = IceFrameSpeed;
	m_framePathList =
	{
		"Asset/Textures/Magic/Ice/Ice0.png",
		"Asset/Textures/Magic/Ice/Ice1.png",
		"Asset/Textures/Magic/Ice/Ice2.png"
	};
	SetFrameTexture(0);
	m_spPoly->SetScale(IceScale);
}

void IceMagic::UpdateChantMagic()
{
	UpdateFrameAnimation();
}

bool IceMagic::IsReadyToFly() const
{
	return m_nowFrame >= 2;
}

void IceMagic::OnBeforeDamage(const std::shared_ptr<EnemyBase>& hitEnemy)
{
	// 通常弾だけ、最初の命中時に1回だけ分散する.
	if (!m_isSplitShot && !m_hasCreatedSplit)
	{
		m_hasCreatedSplit = CreateSplit(hitEnemy);
	}
}

bool IceMagic::ShouldKeepFlyingAfterHit(const std::shared_ptr<EnemyBase>&)
{
	if (m_isSplitShot)
	{
		// 派生弾は低ダメージなので、命中しても寿命まで残す.
		return true;
	}

	if (m_hasCreatedSplit)
	{
		// 通常弾が分散弾を作った後は、親弾を残さず消す.
		return false;
	}

	--m_pierceCount;
	return m_pierceCount > 0;
}

const char* IceMagic::GetShotSoundPath() const
{
	return GetRandomSoundPath(IceShotSoundPathList, g_lastIceShotSoundIndex);
}

const char* IceMagic::GetHitSoundPath() const
{
	return GetRandomSoundPath(IceHitSoundPathList, g_lastIceHitSoundIndex);
}

bool IceMagic::CreateSplit(const std::shared_ptr<EnemyBase>& hitEnemy)
{
	if (m_isSplitShot) { return false; }
	if (m_splitCount <= 0) { return false; }
	if (!hitEnemy) { return false; }
	if (m_dir.LengthSquared() <= 0.0001f) { return false; }

	Math::Vector3 baseDir = m_dir;
	// 命中位置の高低差を引き継ぐと地面へ潜るため、分散弾は水平に飛ばす.
	baseDir.y = 0.0f;
	if (baseDir.LengthSquared() <= 0.0001f) { return false; }
	baseDir.Normalize();

	Math::Vector3 sideDir = { baseDir.z, 0.0f, -baseDir.x };
	if (sideDir.LengthSquared() <= 0.0001f)
	{
		sideDir = { 1.0f, 0.0f, 0.0f };
	}
	else
	{
		sideDir.Normalize();
	}

	for (int i = 0; i < m_splitCount; ++i)
	{
		// 弾数に関係なく、前方90度の扇形と一定の横幅へ均等に配置する.
		const float splitRate = (m_splitCount <= 1)
			? 0.5f
			: static_cast<float>(i) / static_cast<float>(m_splitCount - 1);
		const float sideRate = splitRate * 2.0f - 1.0f;
		const float angle = (splitRate - 0.5f) * IceSplitTotalSpreadAngle;
		const Math::Vector3 splitDir = RotateDirY(baseDir, angle);
		const Math::Vector3 startPos = m_pos + baseDir * IceSplitForwardOffset + sideDir * (sideRate * IceSplitSideOffset);

		std::shared_ptr<IceMagic> spSplitMagic = std::make_shared<IceMagic>();
		spSplitMagic->Shot
		(
			startPos,
			splitDir,
			MagicType::Ice,
			m_damage * 0.5f,
			m_speed,
			nullptr,
			nullptr,
			hitEnemy,
			1,
			0,
			true
		);

		SceneManager::Instance().AddObject(spSplitMagic);
	}

	return true;
}

Math::Vector3 IceMagic::RotateDirY(const Math::Vector3& dir, float angle) const
{
	const float cosAngle = cosf(angle);
	const float sinAngle = sinf(angle);

	Math::Vector3 ret;
	ret.x = dir.x * cosAngle + dir.z * sinAngle;
	ret.y = dir.y;
	ret.z = -dir.x * sinAngle + dir.z * cosAngle;

	if (ret.LengthSquared() > 0.0001f)
	{
		ret.Normalize();
	}

	return ret;
}
