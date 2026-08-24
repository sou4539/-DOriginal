#include "Tree.h"

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
	constexpr float TreeShadowHeight = 0.025f;
	const Math::Color TreeShadowColor = { 0.0f, 0.0f, 0.0f, 0.18f };
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
}

void Tree::MaintainTreesAroundTarget()
{
	if (static_cast<int>(m_trees.size()) >= m_maxTreeCount) { return; }

	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
	if (!spTarget) { return; }

	Math::Vector3 targetPos = spTarget->GetPos();
	targetPos.y = 0.0f;

	Math::Vector3 toLastCenter = targetPos - m_lastAddTreeCenter;
	toLastCenter.y = 0.0f;
	if (toLastCenter.LengthSquared() < m_addTreeInterval * m_addTreeInterval) { return; }

	m_lastAddTreeCenter = targetPos;

	// プレイヤーが移動した先の周辺に木を補充して、草原が寂しくならないようにする。
	CreateRandomTreesInArea
	(
		targetPos.x - m_addTreeAreaHalfSize,
		targetPos.x + m_addTreeAreaHalfSize,
		targetPos.z - m_addTreeAreaHalfSize,
		targetPos.z + m_addTreeAreaHalfSize,
		12,
		180
	);
}

void Tree::AddTree(const Math::Vector3& pos, float angle, float scale, int modelIndex)
{
	TreeData tree;
	tree.pos = pos;
	tree.angle = angle;
	tree.scale = scale;
	tree.modelIndex = modelIndex;
	tree.world =
		Math::Matrix::CreateScale(scale) *
		Math::Matrix::CreateRotationY(angle) *
		Math::Matrix::CreateTranslation(pos);

	m_trees.push_back(tree);

	if (m_pCollider)
	{
		const std::string name = "Tree_" + std::to_string(m_trees.size());
		m_pCollider->RegisterCollisionShape
		(
			name,
			Math::Vector3(pos.x, pos.y + 1.0f, pos.z),
			m_treeRadius * scale,
			KdCollider::TypeGround
		);
	}
}

bool Tree::CanPlaceTree(const Math::Vector3& pos) const
{
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

void Tree::DrawLit()
{
	for (const TreeData& tree : m_trees)
	{
		if (tree.modelIndex < 0 || tree.modelIndex >= static_cast<int>(m_treeModels.size())) { continue; }
		if (!m_treeModels[tree.modelIndex]) { continue; }

		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_treeModels[tree.modelIndex], tree.world);
	}
}

void Tree::DrawEffect()
{
	for (const TreeData& tree : m_trees)
	{
		if (tree.modelIndex < 0 || tree.modelIndex >= static_cast<int>(m_treeModels.size())) { continue; }
		if (!m_treeModels[tree.modelIndex]) { continue; }

		// 木モデルを地面に薄く潰して、足元に見える簡易影として描く。
		Math::Matrix shadowMat = tree.world * Math::Matrix::CreateScale(1.0f, 0.0f, 1.0f);
		shadowMat.Translation({ tree.pos.x, TreeShadowHeight, tree.pos.z });

		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_treeModels[tree.modelIndex], shadowMat, TreeShadowColor);
	}
}

void Tree::GenerateDepthMapFromLight()
{
	for (const TreeData& tree : m_trees)
	{
		if (tree.modelIndex < 0 || tree.modelIndex >= static_cast<int>(m_treeModels.size())) { continue; }
		if (!m_treeModels[tree.modelIndex]) { continue; }

		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_treeModels[tree.modelIndex], tree.world);
	}
}
