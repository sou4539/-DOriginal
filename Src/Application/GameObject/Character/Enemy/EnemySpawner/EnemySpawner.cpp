#include "EnemySpawner.h"

#include "../Bat/Bat.h"
#include "../../Player/Player.h"
#include "../../../../Scene/SceneManager.h"
#include <algorithm>

void EnemySpawner::Init()
{
	m_pDebugWire = std::make_unique<KdDebugWireFrame>();
}

void EnemySpawner::Update()
{
	// インスタンシング描画34
	Bat::UpdateSharedAnimation();

	if (m_pDebugWire)
	{
		std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
		if (spTarget)
		{
			Math::Vector3 debugCenter = spTarget->GetPos();
			debugCenter.y = 3.0f;
			m_pDebugWire->AddDebugSphere(debugCenter, m_spawnMaxRadius, kWhiteColor);
			m_pDebugWire->AddDebugSphere(debugCenter, m_spawnMinRadius, kRedColor);
		}
	}

	UpdateMaxEnemyCountByDistance();
	MaintainEnemyCount();
	UpdateActiveEnemies();
}

void EnemySpawner::SetTarget(const std::shared_ptr<KdGameObject>& target)
{
	m_wpTarget = target;
}

void EnemySpawner::AddSpawnArea(const Math::Vector3& center, float radius, int count)
{
	SpawnArea area;
	area.center = center;
	area.radius = radius;
	area.count = count;

	m_spawnAreas.push_back(area);
}

void EnemySpawner::AddEnemiesToScene(std::list<std::shared_ptr<KdGameObject>>& objList, const std::shared_ptr<KdGameObject>& target)
{
	SetTarget(target);
	m_pObjList = &objList;
	m_enemies.clear();
	UpdateMaxEnemyCountByDistance();

	for (int areaIndex = 0; areaIndex < static_cast<int>(m_spawnAreas.size()); ++areaIndex)
	{
		const SpawnArea& area = m_spawnAreas[areaIndex];

		for (int i = 0; i < area.count; ++i)
		{
			if (GetTotalEnemyCount() >= m_enemyLimit) { return; }

			AddEnemyToScene(objList, target, areaIndex);
		}
	}
}

void EnemySpawner::UpdateMaxEnemyCountByDistance()
{
	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
	if (!spTarget) { return; }

	Math::Vector3 toPlayer = spTarget->GetPos() - m_safeAreaCenter;
	toPlayer.y = 0.0f;

	float distance = toPlayer.Length() - m_safeAreaRadius;
	distance = std::max(distance, 0.0f);

	float rate = distance / m_fullSpawnDistance;
	rate = std::clamp(rate, 0.0f, 1.0f);

	m_enemyLimit = static_cast<int>(m_minEnemyCount + (m_maxEnemyCount - m_minEnemyCount) * rate);
}

Math::Vector3 EnemySpawner::MakeRandomPos(int areaIndex) const
{
	if (areaIndex < 0 || areaIndex >= static_cast<int>(m_spawnAreas.size()))
	{
		return Math::Vector3::Zero;
	}

	const SpawnArea& area = m_spawnAreas[areaIndex];
	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
	if (!spTarget)
	{
		return area.center;
	}

	const Math::Vector3 targetPos = spTarget->GetPos();
	Math::Vector3 pos = targetPos;
	Math::Vector3 areaDir = area.center - m_safeAreaCenter;
	areaDir.y = 0.0f;

	float baseAngle = KdRandom::GetFloat(0.0f, DirectX::XM_2PI);
	if (areaDir.LengthSquared() > 0.0001f)
	{
		baseAngle = std::atan2(areaDir.z, areaDir.x);
	}

	for (int i = 0; i < 30; ++i)
	{
		float angle = baseAngle + KdRandom::GetFloat
		(
			-DirectX::XMConvertToRadians(18.0f),
			DirectX::XMConvertToRadians(18.0f)
		);

		float radiusRate = std::sqrt(KdRandom::GetFloat(0.0f, 1.0f));
		float radius = m_spawnMinRadius + (m_spawnMaxRadius - m_spawnMinRadius) * radiusRate;

		pos.x = targetPos.x + std::cos(angle) * radius;
		pos.y = area.center.y;
		pos.z = targetPos.z + std::sin(angle) * radius;

		if (!IsInSafeArea(pos))
		{
			return pos;
		}
	}

	pos.x = targetPos.x - m_spawnMaxRadius;
	pos.y = area.center.y;
	pos.z = targetPos.z;

	return pos;
}

bool EnemySpawner::IsInSafeArea(const Math::Vector3& pos) const
{
	if (m_safeAreaRadius <= 0.0f) { return false; }

	Math::Vector3 toPos = pos - m_safeAreaCenter;
	toPos.y = 0.0f;

	return toPos.LengthSquared() <= m_safeAreaRadius * m_safeAreaRadius;
}

bool EnemySpawner::IsOutsideSpawnSphere(const Math::Vector3& pos) const
{
	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
	if (!spTarget) { return false; }

	Math::Vector3 toPos = pos - spTarget->GetPos();
	toPos.y = 0.0f;

	return toPos.LengthSquared() > m_spawnMaxRadius * m_spawnMaxRadius;
}

void EnemySpawner::MaintainEnemyCount()
{
	std::vector<EnemyInfo> liveEnemies;

	for (EnemyInfo& enemyInfo : m_enemies)
	{
		std::shared_ptr<EnemyBase> spEnemy = enemyInfo.enemy.lock();
		if (!spEnemy) { continue; }
		if (spEnemy->IsExpired()) { continue; }

		if (IsOutsideSpawnSphere(spEnemy->GetPos()))
		{
			spEnemy->Expire();
			continue;
		}

		liveEnemies.push_back(enemyInfo);
	}

	m_enemies = liveEnemies;

	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
	if (!m_pObjList || !spTarget) { return; }

	if (GetTotalEnemyCount() >= m_enemyLimit) { return; }

	for (int areaIndex = 0; areaIndex < static_cast<int>(m_spawnAreas.size()); ++areaIndex)
	{
		int areaEnemyCount = 0;
		for (const EnemyInfo& enemyInfo : m_enemies)
		{
			if (enemyInfo.areaIndex == areaIndex)
			{
				++areaEnemyCount;
			}
		}

		if (areaEnemyCount < m_spawnAreas[areaIndex].count)
		{
			AddEnemyToScene(*m_pObjList, spTarget, areaIndex);

			if (m_isSpawnOnePerFrame)
			{
				return;
			}
		}
	}
}

void EnemySpawner::UpdateActiveEnemies()
{
	std::vector<std::weak_ptr<EnemyBase>> activeEnemies;

	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
	if (!spTarget)
	{
		SceneManager::Instance().SetActiveEnemies(activeEnemies);
		return;
	}

	const float activeRadiusSq = m_activeEnemyRadius * m_activeEnemyRadius;
	const Math::Vector3 targetPos = spTarget->GetPos();

	for (const EnemyInfo& enemyInfo : m_enemies)
	{
		std::shared_ptr<EnemyBase> spEnemy = enemyInfo.enemy.lock();
		if (!spEnemy) { continue; }
		if (spEnemy->IsExpired()) { continue; }

		Math::Vector3 toEnemy = spEnemy->GetPos() - targetPos;
		toEnemy.y = 0.0f;
		if (toEnemy.LengthSquared() > activeRadiusSq) { continue; }

		activeEnemies.push_back(spEnemy);
	}

	SceneManager::Instance().SetActiveEnemies(activeEnemies);
}

void EnemySpawner::AddEnemyToScene(std::list<std::shared_ptr<KdGameObject>>& objList, const std::shared_ptr<KdGameObject>& target, int areaIndex)
{
	Math::Vector3 startPos = MakeRandomPos(areaIndex);

	std::shared_ptr<Bat> bat = std::make_shared<Bat>();
	bat->SetStartPos(startPos);
	bat->SetAngle(KdRandom::GetFloat(0.0f, DirectX::XM_2PI));
	bat->SetTarget(target);

	std::shared_ptr<Status> spStatus = m_wpStatus.lock();
	if (spStatus)
	{
		bat->SetStatus(spStatus);
	}

	objList.push_back(bat);

	EnemyInfo enemyInfo;
	enemyInfo.enemy = bat;
	enemyInfo.areaIndex = areaIndex;
	m_enemies.push_back(enemyInfo);
}
