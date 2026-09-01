#include "VoltMagic.h"

#include "../../../../Scene/SceneManager.h"
#include "../../Enemy/EnemyBase.h"

namespace
{
	constexpr float VoltScale = 4.0f;
	constexpr float VoltFrameSpeed = 0.20f;
	constexpr float VoltHitRadius = VoltScale * 0.5f;
	constexpr int VoltSoundCount = 4;

	int g_lastShotSoundIndex = -1;
	int g_lastHitSoundIndex = -1;

	const char* VoltShotSoundPathList[VoltSoundCount] =
	{
		"Asset/Sounds/Magic/VoltMagic/Shot/Elec_shot_01.wav",
		"Asset/Sounds/Magic/VoltMagic/Shot/Elec_shot_02.wav",
		"Asset/Sounds/Magic/VoltMagic/Shot/Elec_shot_03.wav",
		"Asset/Sounds/Magic/VoltMagic/Shot/Elec_shot_04.wav"
	};

	const char* VoltHitSoundPathList[VoltSoundCount] =
	{
		"Asset/Sounds/Magic/VoltMagic/Hit/Elec_hit_01.wav",
		"Asset/Sounds/Magic/VoltMagic/Hit/Elec_hit_02.wav",
		"Asset/Sounds/Magic/VoltMagic/Hit/Elec_hit_03.wav",
		"Asset/Sounds/Magic/VoltMagic/Hit/Elec_hit_04.wav"
	};

	const char* GetRandomSoundPath(const char* const soundPathList[VoltSoundCount], int& lastIndex)
	{
		int index = KdRandom::GetInt(0, VoltSoundCount - 2);
		if (lastIndex >= 0 && index >= lastIndex)
		{
			++index;
		}

		lastIndex = index;
		return soundPathList[index];
	}
}

void VoltMagic::Shot(
	const Math::Vector3& startPos,
	const Math::Vector3& dir,
	MagicType type,
	float damage,
	float speed,
	const std::shared_ptr<KdGameObject>& chantTarget,
	const std::shared_ptr<KdGameObject>& flyTarget,
	const std::shared_ptr<KdGameObject>& ignoreTarget,
	int chainCount,
	bool isChainShot)
{
	m_chainCount = chainCount;
	m_isChainShot = isChainShot;

	// 共通の発射初期化はBaseへ任せ、雷専用の連鎖情報だけこのクラスで持つ.
	MagicBase::Shot(startPos, dir, type, damage, speed, chantTarget, flyTarget, ignoreTarget);

	if (m_isChainShot)
	{
		// 連鎖弾は詠唱せず、生成された瞬間から次の敵へ飛ばす.
		StartFly();
		UpdateWorldMatrix();
	}
}

void VoltMagic::SetupMagic()
{
	MagicBase::SetupMagic();

	m_lifeTime = 60.0f;
	m_radius = VoltHitRadius;
	m_frameSpeed = VoltFrameSpeed;

	if (m_isChainShot)
	{
		m_framePathList =
		{
			"Asset/Textures/Magic/Volt/Volt0.png",
			"Asset/Textures/Magic/Volt/Volt1.png",
			"Asset/Textures/Magic/Volt/Volt2.png",
			"Asset/Textures/Magic/Volt/Volt3.png"
		};
	}
	else
	{
		m_framePathList =
		{
			"Asset/Textures/Magic/Volt/Lightning0.png",
			"Asset/Textures/Magic/Volt/Lightning1.png",
			"Asset/Textures/Magic/Volt/Lightning2.png",
			"Asset/Textures/Magic/Volt/Lightning3.png",
			"Asset/Textures/Magic/Volt/Lightning4.png",
			"Asset/Textures/Magic/Volt/Lightning5.png",
			"Asset/Textures/Magic/Volt/Lightning6.png",
			"Asset/Textures/Magic/Volt/Lightning7.png"
		};
	}

	SetFrameTexture(0);
	m_spPoly->SetScale(VoltScale);
}

void VoltMagic::UpdateChantMagic()
{
	UpdateFrameAnimation();
}

void VoltMagic::UpdateFlyMagic()
{
	UpdateFrameAnimation();
}

void VoltMagic::OnAfterDamage(const std::shared_ptr<EnemyBase>& hitEnemy)
{
	// 命中後に残り連鎖回数があれば、次の敵へ雷をつなげる.
	AddChainHitObject(hitEnemy);
	CreateChain(hitEnemy);
}

const char* VoltMagic::GetShotSoundPath() const
{
	return GetRandomSoundPath(VoltShotSoundPathList, g_lastShotSoundIndex);
}

const char* VoltMagic::GetHitSoundPath() const
{
	return GetRandomSoundPath(VoltHitSoundPathList, g_lastHitSoundIndex);
}

Math::Vector3 VoltMagic::GetEmissiveColor() const
{
	return { 0.12f, 0.30f, 0.75f };
}

void VoltMagic::CreateChain(const std::shared_ptr<EnemyBase>& hitEnemy)
{
	if (m_chainCount <= 0) { return; }
	if (!hitEnemy) { return; }

	std::shared_ptr<EnemyBase> spNextTarget = SearchChainTarget(hitEnemy);
	if (!spNextTarget) { return; }

	Math::Vector3 startPos = hitEnemy->GetPos();
	Math::Vector3 dir = spNextTarget->GetPos() - startPos;
	if (dir.LengthSquared() <= 0.0001f) { return; }
	dir.Normalize();

	// 直前に当たった敵へすぐ再ヒットしないよう、少しだけ前へずらす.
	startPos += dir * (m_radius + 0.2f);

	std::shared_ptr<VoltMagic> spChainMagic = std::make_shared<VoltMagic>();
	spChainMagic->m_chainHitList = m_chainHitList;
	spChainMagic->Shot
	(
		startPos,
		dir,
		MagicType::Volt,
		m_damage,
		m_speed,
		nullptr,
		spNextTarget,
		hitEnemy,
		m_chainCount - 1,
		true
	);

	SceneManager::Instance().AddObject(spChainMagic);
}

std::shared_ptr<EnemyBase> VoltMagic::SearchChainTarget(const std::shared_ptr<EnemyBase>& hitEnemy)
{
	if (!hitEnemy) { return nullptr; }

	std::shared_ptr<EnemyBase> spTarget = nullptr;
	const Math::Vector3 hitPos = hitEnemy->GetPos();
	float minDistanceSqr = m_chainRadius * m_chainRadius;

	for (const std::weak_ptr<EnemyBase>& wpEnemy : SceneManager::Instance().GetActiveEnemies())
	{
		std::shared_ptr<EnemyBase> spEnemy = wpEnemy.lock();
		if (!spEnemy) { continue; }
		if (spEnemy == hitEnemy) { continue; }
		if (spEnemy == m_wpIgnoreTarget.lock()) { continue; }
		if (HasChainHitObject(spEnemy)) { continue; }
		if (spEnemy->IsExpired()) { continue; }

		const Math::Vector3 toEnemy = spEnemy->GetPos() - hitPos;
		const float distanceSqr = toEnemy.LengthSquared();
		if (distanceSqr < minDistanceSqr)
		{
			minDistanceSqr = distanceSqr;
			spTarget = spEnemy;
		}
	}

	return spTarget;
}

bool VoltMagic::HasChainHitObject(const std::shared_ptr<KdGameObject>& obj) const
{
	if (!obj) { return false; }

	for (const std::weak_ptr<KdGameObject>& wpHitObj : m_chainHitList)
	{
		if (wpHitObj.lock() == obj)
		{
			return true;
		}
	}

	return false;
}

void VoltMagic::AddChainHitObject(const std::shared_ptr<KdGameObject>& obj)
{
	if (!obj) { return; }
	if (HasChainHitObject(obj)) { return; }

	m_chainHitList.push_back(obj);
}
