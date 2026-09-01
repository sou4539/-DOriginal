#include "Village.h"

void Village::Init()
{
	// 村モデルと同じ形の当たり判定を作る。
	m_spModel = std::make_shared<KdModelWork>();
	m_spModel->SetModelData("Asset/Models/Objects/Stage/World/village.gltf");

	m_mWorld = Math::Matrix::CreateTranslation(0.0f, 0.0f, 0.0f);

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

	// 村の安全地帯を青いデバッグスフィアで表示する。
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

void Village::GenerateDepthMapFromLight()
{
	if (!IsInVisibleRange()) { return; }
	if (!m_spModel) { return; }

	// 村が表示されている時だけ影も作り、非表示中に影だけ残らないようにする。
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}
void Village::DrawEffect()
{
	if (!IsInVisibleRange()) { return; }
	if (!m_spModel) { return; }

	// Fake ground shadow.
	Math::Matrix shadowMat = m_mWorld * Math::Matrix::CreateScale(1.0f, 0.0f, 1.0f);
	shadowMat.Translation({ 0.0f, 0.025f, 0.0f });

	const Math::Color shadowColor = { 0.0f, 0.0f, 0.0f, 0.20f };
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, shadowMat, shadowColor);
}
bool Village::IsInVisibleRange() const
{
	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
	if (!spTarget) { return true; }

	Math::Vector3 toVillage = m_safeAreaCenter - spTarget->GetPos();
	toVillage.y = 0.0f;

	return toVillage.LengthSquared() <= m_visibleRadius * m_visibleRadius;
}


