#include "StaffBase.h"

void StaffBase::Init()
{
}

void StaffBase::Update()
{
	auto spTarget = m_wpTarget.lock();

	if (!spTarget)
	{
		return;
	}

	m_angle += m_rotateSpeed;

	float x = cos(m_angle) * m_radius;
	float z = sin(m_angle) * m_radius;

	m_pos = spTarget->GetPos() + Math::Vector3(x, m_height, z);

	m_mWorld = Math::Matrix::CreateTranslation(m_pos);
}