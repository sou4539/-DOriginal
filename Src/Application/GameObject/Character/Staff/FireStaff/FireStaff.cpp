#include "FireStaff.h"

void FireStaff::Init()
{
	m_spModel = std::make_shared<KdModelWork>();
	m_spModel->SetModelData("Asset/Models/Objects/Character/Staff/FireStaff.gltf");

	// FireStaff用の魔法性能を設定する。
	SetMagicParam(MagicType::Fire, 5.0f, 0.2f, 60.0f);
}

void FireStaff::Update()
{
	StaffBase::Update();
}
