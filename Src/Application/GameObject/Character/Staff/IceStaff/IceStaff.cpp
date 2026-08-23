#include "IceStaff.h"

void IceStaff::Init()
{
	m_spModel = std::make_shared<KdModelWork>();
	m_spModel->SetModelData("Asset/Models/Objects/Character/Staff/IceStaff.gltf");

	// IceStaff用の魔法性能を設定する。

	SetMagicParam(MagicType::Ice, 2.0f, 0.15f, 90.0f);
}

void IceStaff::Update()
{
	StaffBase::Update();
}
