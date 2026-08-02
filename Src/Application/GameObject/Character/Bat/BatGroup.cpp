#include "BatGroup.h"

#include "../../../Scene/SceneManager.h"

void BatGroup::Init()
{
	m_spBat = std::make_shared<Bat>();
	SceneManager::Instance().AddObject(m_spBat);

	m_spBat->SetStartPos({ -15,3,0 });
}

void BatGroup::Update()
{
}