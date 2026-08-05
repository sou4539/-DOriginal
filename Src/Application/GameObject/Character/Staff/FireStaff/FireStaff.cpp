#include "FireStaff.h"

void FireStaff::Init()
{
	m_spModel = std::make_shared<KdModelWork>();
	m_spModel->SetModelData("Asset/Models/Objects/Character/Staff/FireStaff.gltf");
}

void FireStaff::Update()
{
	StaffBase::Update();
}