#include "MagicBase.h"

void MagicBase::Init()
{
	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

	m_pos = {};
	m_dir = {};
	m_magicType = MagicType::None;
	m_damage = 0.0f;
	m_speed = 0.0f;
	m_lifeTime = 0.0f;
	m_radius = 0.0f;
}

void MagicBase::Update()
{
	m_pos += m_dir * m_speed;
	m_lifeTime -= 1.0f;

	if (m_lifeTime <= 0.0f)
	{
		m_isExpired = true;
	}

	if (m_pDebugWire)
	{
		m_pDebugWire->AddDebugSphere(m_pos, m_radius);
	}

	m_mWorld = Math::Matrix::CreateTranslation(m_pos);
}

void MagicBase::DrawLit()
{}

void MagicBase::Shot(const Math::Vector3& startPos, const Math::Vector3& dir, MagicType type, float damage, float speed)
{
	m_pos = startPos;
	m_dir = dir;
	m_magicType = type;
	m_damage = damage;
	m_speed = speed;

	m_dir.Normalize();

	switch (m_magicType)
	{
	case MagicType::Fire:
		m_lifeTime = 90;
		m_radius = 0.5f;
		break;
	case MagicType::Ice:
		m_lifeTime = 120;
		m_radius = 0.3f;
		break;
	case MagicType::Volt:
		m_lifeTime = 60;
		m_radius = 0.2f;
		break;
	default:
		break;
	}
}