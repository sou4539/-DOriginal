#pragma once

#include "../CharaBase.h"
#include "Bat.h"

class Bat;

class BatGroup :public CharaBase
{
public:
	BatGroup() { Init(); }
	~BatGroup() {}
	void Init();
	void Update();

	void SetTarget(const std::shared_ptr<KdGameObject>& target)
	{
		if (m_spBat)
		{
			m_spBat->SetTarget(target);
		}
	}

private:
	std::shared_ptr<Bat> m_spBat;
};