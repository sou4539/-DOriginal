#include "Village.h"

void Village::Init()
{
	// 村モデルと当たり判定を準備する。
	m_spModel = std::make_shared<KdModelWork>();
	m_spModel->SetModelData("Asset/Models/Objects/Stage/World/village.gltf");

	m_mWorld = Math::Matrix::CreateTranslation(0.120777f, -0.878495f, 14.227562f);

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape
	(
		"Village",
		m_spModel,
		KdCollider::TypeGround
	);
}

void Village::Update()
{
	if (!IsInVisibleRange()) { return; }

	// 村の安全地帯を青いスフィアで表示する。
	if (m_pDebugWire)
	{
		m_pDebugWire->AddDebugSphere(m_safeAreaCenter, m_safeAreaRadius, kBlueColor);
	}
}

void Village::DrawLit()
{
	if (!IsInVisibleRange()) { return; }

	StageBase::DrawLit();
}

bool Village::IsInVisibleRange() const
{
	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
	if (!spTarget) { return true; }

	Math::Vector3 toVillage = m_safeAreaCenter - spTarget->GetPos();
	toVillage.y = 0.0f;

	return toVillage.LengthSquared() <= m_visibleRadius * m_visibleRadius;
}


