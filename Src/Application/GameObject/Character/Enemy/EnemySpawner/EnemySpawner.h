#pragma once

#include "../EnemyBase.h"
#include <vector>

class Bat;
class Status;

class EnemySpawner : public CharaBase
{
public:
	EnemySpawner() { Init(); }
	~EnemySpawner() override {}

	void Init() override;
	void Update() override;

	void AddSpawnArea(const Math::Vector3& center, float radius, int count);
	void SetTarget(const std::shared_ptr<KdGameObject>& target);

	void SetStatus(const std::shared_ptr<Status>& status)
	{
		m_wpStatus = status;
	}

	void SetSafeArea(const Math::Vector3& center, float radius)
	{
		m_safeAreaCenter = center;
		m_safeAreaRadius = radius;
	}

	void AddEnemiesToScene(std::list<std::shared_ptr<KdGameObject>>& objList, const std::shared_ptr<KdGameObject>& target);

private:
	struct SpawnArea
	{
		Math::Vector3 center = Math::Vector3::Zero;
		float radius = 0.0f;
		int count = 0;
	};

	struct EnemyInfo
	{
		std::weak_ptr<EnemyBase> enemy;
		int spawnAreaIndex = 0;
	};

	void UpdateMaxEnemyCountByDistance();
	void MaintainEnemyCount();
	void UpdateActiveEnemies();

	Math::Vector3 MakeRandomPos(int spawnAreaIndex) const;
	bool IsInSafeArea(const Math::Vector3& pos) const;
	bool IsOutsideSpawnSphere(const Math::Vector3& pos) const;

	int GetTotalEnemyCount() const
	{
		return static_cast<int>(m_enemies.size());
	}

	void AddEnemyToScene(std::list<std::shared_ptr<KdGameObject>>& objList, const std::shared_ptr<KdGameObject>& target, int spawnAreaIndex);

	std::vector<SpawnArea> m_spawnAreas;
	std::vector<EnemyInfo> m_enemies;

	std::list<std::shared_ptr<KdGameObject>>* m_pObjList = nullptr;
	std::weak_ptr<KdGameObject> m_wpTarget;
	std::weak_ptr<Status> m_wpStatus;

	float m_spawnMinRadius = 55.0f;
	float m_spawnMaxRadius = 70.0f;

	Math::Vector3 m_safeAreaCenter = Math::Vector3::Zero;
	float m_safeAreaRadius = 0.0f;

	bool m_isSpawnOnePerFrame = true;

	int m_nowMaxEnemyCount = 1;
	int m_minEnemyCount = 1;
	int m_maxEnemyCount = 100;
	float m_maxEnemyCountDistance = 250.0f;
	float m_activeEnemyRadius = 100.0f;
};