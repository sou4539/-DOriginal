#pragma once

#include "../StageBase.h"

#include <array>
#include <string>
#include <vector>

class Tree : public StageBase
{
public:
	Tree() { Init(); }
	~Tree() override {}

	void Update() override;
	void DrawLit() override;
	void GenerateDepthMapFromLight() override;

	void SetTarget(const std::shared_ptr<KdGameObject>& target) { m_wpTarget = target; }

	// 木の幹を壁として扱うため、キャラの球判定も有効にする.
	bool EnableSphereCollision() const override { return true; }

private:
	struct TreeData
	{
		Math::Vector3 pos = Math::Vector3::Zero;
		float angle = 0.0f;
		float scale = 1.0f;
		int modelIndex = 0;
		Math::Matrix world = Math::Matrix::Identity;
		std::string colliderName;
		bool hasCollider = false;
	};

	struct AvoidCircle
	{
		Math::Vector3 center = Math::Vector3::Zero;
		float radius = 0.0f;
	};

	void Init() override;
	void LoadModels();
	void CreateRandomTrees();
	void CreateRandomTreesInArea(float minX, float maxX, float minZ, float maxZ, int addCount, int tryCount);
	void MaintainTreesAroundTarget();
	int	 RemoveTreesOutsideActiveRange();
	void UpdateTreeColliders();
	void AddTree(const Math::Vector3& pos, float angle, float scale, int modelIndex);
	void RegisterTreeCollider(TreeData& tree);
	void RemoveTreeCollider(TreeData& tree);
	bool CanPlaceTree(const Math::Vector3& pos) const;
	bool IsInCircle(const Math::Vector3& pos, const AvoidCircle& circle) const;
	bool IsNearTarget(const Math::Vector3& pos, float radius) const;
	bool IsOutsideActiveRange(const Math::Vector3& pos) const;
	bool IsTooCloseToTarget(const Math::Vector3& pos) const;

	std::weak_ptr<KdGameObject> m_wpTarget;
	std::array<std::shared_ptr<KdModelWork>, 3> m_treeModels;
	std::vector<TreeData> m_trees;
	std::vector<AvoidCircle> m_avoidCircles;

	int m_treeCount = 35;
	int m_maxTreeCount = 50;
	int m_nextTreeId = 0;
	float m_treeRadius = 1.1f;
	float m_minTreeDistance = 5.0f;
	float m_minCreateDistanceFromTarget = 70.0f;
	float m_activeRadius = 120.0f;
	float m_shadowDrawRadius = 55.0f;
	float m_colliderActiveRadius = 45.0f;
	float m_addTreeAreaHalfSize = 100.0f;
};
