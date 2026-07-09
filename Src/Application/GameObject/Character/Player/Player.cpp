#include "Player.h"

void Player::Init()
{
	if (!m_spPoly)
	{
		m_spPoly = std::make_shared<KdSquarePolygon>();
		m_spPoly->SetMaterial("Asset/Data/LessonData/Character/Hamu.png");
		m_spPoly->SetPivot(KdSquarePolygon::PivotType::Center_Bottom);
	}

	m_pos = { -12.0f, 2.5f, 1.5f };

	//SetPos(m_pos);
}

void Player::Update()
{
	Math::Matrix m_trans = Math::Matrix::CreateTranslation(m_pos);
	m_mWorld = m_trans;
}

void Player::PostUpdate()
{}