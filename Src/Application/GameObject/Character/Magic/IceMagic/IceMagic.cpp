#include "IceMagic.h"

#include "../../../../Scene/SceneManager.h"
#include "../../Enemy/EnemyBase.h"

namespace
{
	// サイズ
	constexpr float IceScale = 4.0f;
	// アニメーション速度
	constexpr float IceFrameSpeed = 0.12f;
	// 当たり判定サイズ
	constexpr float IceHitRadius = IceScale * 0.5f;
	// 分散数の最小値
	constexpr int IceMinSplitCount = 0;
	// 分散数の最大値
	constexpr int IceMaxSplitCount = 8;
	constexpr float IceSpreadAngle = DirectX::XMConvertToRadians(90.0f);
	constexpr float IceSplitForwardOffset = 1.5f;
	constexpr float IceSplitSideOffset = 1.6f;
	// 音の種類数
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

bool IceMagic::ConfigureShot(const MagicShotParams& params)
{
	// 氷専用の値はIceMagic自身に保存する。
	m_pierceLeft = params.pierceCount;
	m_isSplitShot = params.isSplitShot;
	// 分散弾からさらに分散しないよう、派生弾の分散数は0に固定する。
	m_splitCount = m_isSplitShot ? 0 : std::clamp(params.splitCount, IceMinSplitCount, IceMaxSplitCount);
	m_hasSplit = false;
	return m_isSplitShot; // Baseが画像設定を終えた後に即時発射する。
}

void IceMagic::SetupMagic()
{
	MagicBase::SetupMagic();

	m_lifeFrames = 120.0f;
	m_radius = IceHitRadius;
	m_frameSpeed = IceFrameSpeed;
	m_framePaths =
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
	return m_frameIndex >= 2;
}

void IceMagic::OnBeforeDamage(const std::shared_ptr<EnemyBase>& hitEnemy)
{
	// 通常弾だけ、最初の命中時に1回だけ分散する.
	if (!m_isSplitShot && !m_hasSplit)
	{
		m_hasSplit = CreateSplit(hitEnemy);
	}
}

bool IceMagic::ShouldKeepFlyingAfterHit(const std::shared_ptr<EnemyBase>&)
{
	if (m_isSplitShot)
	{
		// 派生弾は低ダメージなので、命中しても寿命まで残す.
		return true;
	}

	if (m_hasSplit)
	{
		// 通常弾が分散弾を作った後は、親弾を残さず消す.
		return false;
	}

	--m_pierceLeft;
	return m_pierceLeft > 0;
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
		const float angle = (splitRate - 0.5f) * IceSpreadAngle;
		const Math::Vector3 splitDir = RotateDirY(baseDir, angle);
		const Math::Vector3 startPos = m_pos + baseDir * IceSplitForwardOffset + sideDir * (sideRate * IceSplitSideOffset);

		std::shared_ptr<IceMagic> spSplitMagic = std::make_shared<IceMagic>();
		// 分散弾も同じ入口を使用。照準先を指定せず、splitDir方向へ即時発射する。
		MagicShotParams params;
		params.startPos = startPos;
		params.dir = splitDir;
		params.damage = m_damage * 0.5f;
		params.speed = m_speed;
		params.ignoreTarget = hitEnemy;
		params.isSplitShot = true;
		spSplitMagic->Shot(params);

		SceneManager::Instance().AddObject(spSplitMagic);
	}

	return true;
}

Math::Vector3 IceMagic::RotateDirY(const Math::Vector3& dir, float angle) const
{
	const float cosAngle = cosf(angle);
	const float sinAngle = sinf(angle);

	Math::Vector3 rotatedDir;
	rotatedDir.x = dir.x * cosAngle + dir.z * sinAngle;
	rotatedDir.y = dir.y;
	rotatedDir.z = -dir.x * sinAngle + dir.z * cosAngle;

	if (rotatedDir.LengthSquared() > 0.0001f)
	{
		rotatedDir.Normalize();
	}

	return rotatedDir;
}
