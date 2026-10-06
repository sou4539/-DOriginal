#include "Castle.h"

void Castle::Init()
{
	m_spModel = std::make_shared<KdModelWork>();
	m_spModel->SetModelData("Asset/Models/Objects/Stage/Castle/Castle.gltf");

	// 位置を調整するときは下の座標を変更する。モデル内のスケールはそのまま使う。
	Math::Matrix mScale = Math::Matrix::CreateScale(3.0f,0.75f,3.0f);
	Math::Matrix mTrans = Math::Matrix::CreateTranslation(-30.0f, 0.0f, 0.0f);
	m_mWorld = mScale * mTrans;
}

void Castle::GenerateDepthMapFromLight()
{
	if (!m_spModel) { return; }
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}
