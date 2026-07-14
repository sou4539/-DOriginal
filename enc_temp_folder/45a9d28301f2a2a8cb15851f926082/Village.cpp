#include "Village.h"

void Village::Update()
{}

void Village::PostUpdate()
{}

void Village::Init()
{
	m_spModel = std::make_shared<KdModelWork>();
	m_spModel->SetModelData("Asset/Models/Objects/World/village.gltf");

	m_mWorld = Math::Matrix::Identity;

	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape
	(
		"Village", 
		m_spModel, 
		KdCollider::TypeGround
	);

}
