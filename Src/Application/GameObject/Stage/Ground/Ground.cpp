#include "Ground.h"

void Ground::Init()
{
	// 地面モデルを読み込む。
	m_spModel = std::make_shared<KdModelWork>();
	m_spModel->SetModelData("Asset/Models/Objects/Ground/Ground.gltf");

	// 元のGroundモデルは小さいため、表示確認しやすいように100倍へ拡大する。
	m_mWorld = Math::Matrix::CreateScale(100.0f);

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
