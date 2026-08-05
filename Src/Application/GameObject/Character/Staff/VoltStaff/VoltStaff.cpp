#include "VoltStaff.h"

void VoltStaff::Init()
{
	m_spModel = std::make_shared<KdModelWork>();
	m_spModel->SetModelData("Asset/Models/Objects/Character/Staff/VoltStaff.gltf");
}

void VoltStaff::Update()
{
	StaffBase::Update();
}