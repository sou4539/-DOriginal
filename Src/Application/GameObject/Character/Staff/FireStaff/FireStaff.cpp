#include "FireStaff.h"

#include "../Magic/MagicBase.h"
#include "../../../../Scene/SceneManager.h"

void FireStaff::Init()
{
	m_spModel = std::make_shared<KdModelWork>();
	m_spModel->SetModelData("Asset/Models/Objects/Character/Staff/FireStaff.gltf");

	MagicCD = 0.0f;

	
}

void FireStaff::Update()
{
	StaffBase::Update();
	auto spTarget = m_wpTarget.lock();
	if (!spTarget)
	{
		return;
	}

	MagicCD--;

	if (MagicCD <= 0.0f)
	{
		std::shared_ptr<MagicBase> magic = std::make_shared<MagicBase>();
		magic->Shot(GetPos(), spTarget->GetDir(), MagicType::Fire, 10.0f, 0.2f);
		SceneManager::Instance().AddObject(magic);

		MagicCD = 60.0f; // 1秒のクールダウン
	}

	
}