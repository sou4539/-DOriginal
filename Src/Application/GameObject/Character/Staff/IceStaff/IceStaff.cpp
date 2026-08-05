#include "IceStaff.h"

void IceStaff::Init()
{
	m_spModel = std::make_shared<KdModelWork>();
	m_spModel->SetModelData("Asset/Models/Objects/Character/Staff/IceStaff.gltf");
}

void IceStaff::Update()
{
	StaffBase::Update();
}