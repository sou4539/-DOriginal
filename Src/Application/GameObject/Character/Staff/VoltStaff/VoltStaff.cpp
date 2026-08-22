#include "VoltStaff.h"

void VoltStaff::Init()
{
	m_spModel = std::make_shared<KdModelWork>();
	m_spModel->SetModelData("Asset/Models/Objects/Character/Staff/VoltStaff.gltf");

	// VoltStaff用の魔法性能を設定する。
	// 攻撃処理本体はStaffBase::Updateで共通して行う。
	SetMagicParam(MagicType::Volt, 3.0f, 0.3f, 30.0f);
}

void VoltStaff::Update()
{
	StaffBase::Update();
}