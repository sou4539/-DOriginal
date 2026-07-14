#include "Village.h"

void Village::Update()
{}

void Village::PostUpdate()
{}

void Village::Init()
{
	// 村モデルを読み込む。
	// モデル内に「COL」という名前のノードがある場合、
	// フレームワーク側でそのノードが当たり判定用として使われる。
	m_spModel = std::make_shared<KdModelWork>();
	m_spModel->SetModelData("Asset/Models/Objects/World/village.gltf");

	// 村は原点・等倍で配置する。
	m_mWorld = Math::Matrix::Identity;

	// 村の当たり判定を登録する。
	// 今はTypeGroundに統一しているため、地面レイ判定も壁用スフィア判定も
	// この1つのコライダーを見に行く。
	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape
	(
		"Village", 
		m_spModel, 
		KdCollider::TypeGround
	);

}
