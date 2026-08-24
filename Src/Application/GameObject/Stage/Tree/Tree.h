#pragma once

#include "../StageBase.h"

#include <array>
#include <vector>

class Tree : public StageBase
{
public:
	Tree() { Init(); }
	~Tree() override {}

	void DrawLit() override;
	void DrawEffect() override;
	void GenerateDepthMapFromLight() override;

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
	};

	struct AvoidCircle
	{
		Math::Vector3 center = Math::Vector3::Zero;
		float radius = 0.0f;
	};

	void Init() override;
	void LoadModels();
	void CreateRandomTrees();
	void AddTree(const Math::Vector3& pos, float angle, float scale, int modelIndex);
	bool CanPlaceTree(const Math::Vector3& pos) const;
	bool IsInCircle(const Math::Vector3& pos, const AvoidCircle& circle) const;

	std::array<std::shared_ptr<KdModelWork>, 3> m_treeModels;
	std::vector<TreeData> m_trees;
	std::vector<AvoidCircle> m_avoidCircles;

	int m_treeCount = 35;
	float m_treeRadius = 1.1f;
	float m_minTreeDistance = 5.0f;
};
