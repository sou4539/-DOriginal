#include "IceStaff.h"

void IceStaff::Init()
{
	m_spModel = std::make_shared<KdModelWork>();
	m_spModel->SetModelData("Asset/Models/Objects/Character/Staff/IceStaff.gltf");

	// IceStaff用の魔法性能を設定する。
	// 攻撃処理本体はStaffBase::Updateで共通して行う。

	SetMagicParam(MagicType::Ice, 7.0f, 0.15f, 90.0f);
}

void IceStaff::Update()
{
	StaffBase::Update();
}
