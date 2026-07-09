#include "Player.h"

void Player::Init()
{
	if (!m_spModel)
	{
		m_spModel = std::make_shared<KdModelWork>();
		m_spModel->SetModelData("Asset/Models/Objects/Witch/Witch.gltf");
	}

	m_pos = {};

	//SetPos(m_pos);
}

void Player::Update()
{
	CharaBase::Update();

	Math::Matrix m_trans = Math::Matrix::CreateTranslation(m_pos);
	m_mWorld = m_trans;
}

void Player::PostUpdate()
{

}