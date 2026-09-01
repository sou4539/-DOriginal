#include "Tree.h"

#include <algorithm>
#include <string>

namespace
{
	const std::array<std::string, 3> TreeModelPaths =
	{
		"Asset/Models/Objects/Stage/Tree/Tree_001.gltf",
		"Asset/Models/Objects/Stage/Tree/Tree_002.gltf",
		"Asset/Models/Objects/Stage/Tree/Tree_003.gltf"
	};

	constexpr float TreeAreaMinX = -140.0f;
	constexpr float TreeAreaMaxX = 80.0f;
	constexpr float TreeAreaMinZ = -95.0f;
	constexpr float TreeAreaMaxZ = 95.0f;
	constexpr int TreePlaceTryCount = 400;
}

void Tree::Init()
{
	LoadModels();

	m_pCollider = std::make_unique<KdCollider>();

	m_avoidCircles =
	{
		{ Math::Vector3::Zero, 52.0f },
		{ { -70.0f, 0.0f,   0.0f }, 22.0f },
		{ { -30.0f, 0.0f,  55.0f }, 20.0f },
		{ { -30.0f, 0.0f, -75.0f }, 20.0f },
		{ {  55.0f, 0.0f, -45.0f }, 20.0f },
		{ { -85.0f, 0.0f,  45.0f }, 20.0f },
		{ { -85.0f, 0.0f, -45.0f }, 20.0f },
		{ { -120.0f, 0.0f,  0.0f }, 22.0f }
	};

	CreateRandomTrees();
}

void Tree::LoadModels()
{
	for (int i = 0; i < static_cast<int>(m_treeModels.size()); ++i)
	{
		m_treeModels[i] = std::make_shared<KdModelWork>();
		m_treeModels[i]->SetModelData(TreeModelPaths[i]);
	}
}

void Tree::CreateRandomTrees()
{
	CreateRandomTreesInArea
	(
		TreeAreaMinX,
		TreeAreaMaxX,
		TreeAreaMinZ,
		TreeAreaMaxZ,
		m_treeCount,
		TreePlaceTryCount
	);
}

void Tree::CreateRandomTreesInArea(float minX, float maxX, float minZ, float maxZ, int addCount, int tryCount)
{
	const int targetTreeCount = std::min(static_cast<int>(m_trees.size()) + addCount, m_maxTreeCount);

	for (int i = 0; i < tryCount && static_cast<int>(m_trees.size()) < targetTreeCount; ++i)
	{
		Math::Vector3 pos;
		pos.x = KdRandom::GetFloat(minX, maxX);
		pos.y = 0.0f;
		pos.z = KdRandom::GetFloat(minZ, maxZ);

		if (!CanPlaceTree(pos)) { continue; }

		const float angle = KdRandom::GetFloat(0.0f, DirectX::XM_2PI);
		const float scale = KdRandom::GetFloat(0.8f, 1.2f);
		const int modelIndex = KdRandom::GetInt(0, static_cast<int>(m_treeModels.size()) - 1);

		AddTree(pos, angle, scale, modelIndex);
	}
}

void Tree::Update()
{
	MaintainTreesAroundTarget();
	UpdateTreeColliders();
}

void Tree::MaintainTreesAroundTarget()
{
	const int removedTreeCount = RemoveTreesOutsideActiveRange();

	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
	if (!spTarget) { return; }
	if (removedTreeCount <= 0) { return; }
	if (static_cast<int>(m_trees.size()) >= m_maxTreeCount) { return; }

	Math::Vector3 targetPos = spTarget->GetPos();
	targetPos.y = 0.0f;

	// 消えた本数だけ補充し、移動距離ではなく削除を基準に木の数を保つ。
	CreateRandomTreesInArea
	(
		targetPos.x - m_addTreeAreaHalfSize,
		targetPos.x + m_addTreeAreaHalfSize,
		targetPos.z - m_addTreeAreaHalfSize,
		targetPos.z + m_addTreeAreaHalfSize,
		removedTreeCount,
		removedTreeCount * 30
	);
}

void Tree::AddTree(const Math::Vector3& pos, float angle, float scale, int modelIndex)
{
	TreeData tree;
	tree.pos = pos;
	tree.angle = angle;
	tree.scale = scale;
	tree.modelIndex = modelIndex;
	tree.colliderName = "Tree_" + std::to_string(m_nextTreeId++);
	tree.world =
		Math::Matrix::CreateScale(scale) *
		Math::Matrix::CreateRotationY(angle) *
		Math::Matrix::CreateTranslation(pos);

	m_trees.push_back(tree);
}

void Tree::RegisterTreeCollider(TreeData& tree)
{
	if (!m_pCollider || tree.hasCollider) { return; }

	m_pCollider->RegisterCollisionShape
	(
		tree.colliderName,
		Math::Vector3(tree.pos.x, tree.pos.y + 1.0f, tree.pos.z),
		m_treeRadius * tree.scale,
		KdCollider::TypeGround
	);

	tree.hasCollider = true;
}

void Tree::RemoveTreeCollider(TreeData& tree)
{
	if (!m_pCollider || !tree.hasCollider) { return; }

	m_pCollider->RemoveCollisionShape(tree.colliderName);
	tree.hasCollider = false;
}

bool Tree::CanPlaceTree(const Math::Vector3& pos) const
{
	if (IsTooCloseToTarget(pos)) { return false; }

	for (const AvoidCircle& circle : m_avoidCircles)
	{
		if (IsInCircle(pos, circle)) { return false; }
	}

	const float minDistanceSqr = m_minTreeDistance * m_minTreeDistance;
	for (const TreeData& tree : m_trees)
	{
		Math::Vector3 toTree = tree.pos - pos;
		toTree.y = 0.0f;
		if (toTree.LengthSquared() < minDistanceSqr)
		{
			return false;
		}
	}

	return true;
}

bool Tree::IsInCircle(const Math::Vector3& pos, const AvoidCircle& circle) const
{
	Math::Vector3 toPos = pos - circle.center;
	toPos.y = 0.0f;

	return toPos.LengthSquared() <= circle.radius * circle.radius;
}

bool Tree::IsNearTarget(const Math::Vector3& pos, float radius) const
{
	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
	if (!spTarget) { return true; }

	Math::Vector3 toPos = pos - spTarget->GetPos();
	toPos.y = 0.0f;

	return toPos.LengthSquared() <= radius * radius;
}

bool Tree::IsTooCloseToTarget(const Math::Vector3& pos) const
{
	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
	if (!spTarget) { return false; }

	// プレイヤーの近くには新しく木を出さず、画面内で突然生える違和感を減らす。
	return IsNearTarget(pos, m_minCreateDistanceFromTarget);
}

int Tree::RemoveTreesOutsideActiveRange()
{
	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
	if (!spTarget) { return 0; }
	if (!m_pCollider) { return 0; }

	int removedTreeCount = 0;

	m_trees.erase
	(
		std::remove_if(m_trees.begin(), m_trees.end(), [this, &removedTreeCount](const TreeData& tree)
		{
			if (!IsOutsideActiveRange(tree.pos)) { return false; }

			// 表示範囲外の木は描画リストと当たり判定の両方から削除する。
			if (tree.hasCollider)
			{
				m_pCollider->RemoveCollisionShape(tree.colliderName);
			}

			++removedTreeCount;
			return true;
		}),
		m_trees.end()
	);

	return removedTreeCount;
}

void Tree::UpdateTreeColliders()
{
	for (TreeData& tree : m_trees)
	{
		if (IsNearTarget(tree.pos, m_colliderActiveRadius))
		{
			RegisterTreeCollider(tree);
		}
		else
		{
			RemoveTreeCollider(tree);
		}
	}
}

bool Tree::IsOutsideActiveRange(const Math::Vector3& pos) const
{
	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
	if (!spTarget) { return false; }

	Math::Vector3 toPos = pos - spTarget->GetPos();
	toPos.y = 0.0f;

	return toPos.LengthSquared() > m_activeRadius * m_activeRadius;
}

void Tree::DrawLit()
{
	for (const TreeData& tree : m_trees)
	{
		if (tree.modelIndex < 0 || tree.modelIndex >= static_cast<int>(m_treeModels.size())) { continue; }
		if (!m_treeModels[tree.modelIndex]) { continue; }

		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_treeModels[tree.modelIndex], tree.world);
	}
}

void Tree::GenerateDepthMapFromLight()
{
	for (const TreeData& tree : m_trees)
	{
		if (tree.modelIndex < 0 || tree.modelIndex >= static_cast<int>(m_treeModels.size())) { continue; }
		if (!m_treeModels[tree.modelIndex]) { continue; }
		if (!IsNearTarget(tree.pos, m_shadowDrawRadius)) { continue; }

		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_treeModels[tree.modelIndex], tree.world);
	}
}
