#include "Castle.h"

void Castle::Init()
{
	m_spModel = std::make_shared<KdModelWork>();
	m_spModel->SetModelData("Asset/Models/Objects/Stage/Castle/Castle.gltf");

	// 位置を調整するときは下の座標を変更する。モデル内のスケールはそのまま使う。
	Math::Matrix mScale = Math::Matrix::CreateScale(3.0f,0.75f,3.0f);
	Math::Matrix mTrans = Math::Matrix::CreateTranslation(-30.0f, 0.0f, 0.0f);
	m_mWorld = mScale * mTrans;

	// 描画と同じモデル・行列を使って壁の当たり判定を作る。
	// TypeGroundはプレイヤーの壁との球判定が調べる属性。
	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape("Castle", m_spModel, KdCollider::TypeGround);
}

void Castle::DrawLit()
{
	if (!m_spModel) { return; }
	auto& shader = KdShaderManager::Instance().m_StandardShader;
	// Fade only walls between the camera and the player.
	if (auto target = m_wpTarget.lock())
	{
		const Math::Vector3 bodyPos = target->GetPos() + Math::Vector3(0.0f, 1.0f, 0.0f);
		shader.SetAlphaDither(true, 0.2f, true, true);
		shader.SetWallDitherTarget(bodyPos, 3.0f);
	}
	shader.DrawModel(*m_spModel, m_mWorld);
}

void Castle::GenerateDepthMapFromLight()
{
	if (!m_spModel) { return; }
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}
