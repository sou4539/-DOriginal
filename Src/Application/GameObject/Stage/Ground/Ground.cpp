#include "Ground.h"

void Ground::Init()
{
	// 地面モデルを読み込む。
	m_spModel = std::make_shared<KdModelWork>();
	m_spModel->SetModelData("Asset/Models/Objects/Stage/Ground/Ground.gltf");

	// 元のGroundモデルは小さいため、ゲームで使いやすい広さまで拡大する。
	// サイズだけ大きくするとテクスチャが引き延ばされるため、
	// DrawLit()側でUVも同じ比率で繰り返す。
	m_mWorld = Math::Matrix::CreateScale(m_groundScale);

	// 地面として使うため、TypeGroundの当たり判定を登録する。
	// CharaBaseの下向きレイ判定とスフィア判定が、このコライダーを参照する。
	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape
	(
		"Ground",
		m_spModel,
		KdCollider::TypeGround
	);
}

void Ground::DrawLit()
{
	if (!m_spModel) { return; }

	// GroundだけUVを繰り返して描画する。
	// これにより、地面を広くしても画像が大きく引き延ばされず、
	// 同じ模様がタイル状に表示される。
	KdShaderManager::Instance().m_StandardShader.SetUVTiling(m_uvTiling * 100);
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

