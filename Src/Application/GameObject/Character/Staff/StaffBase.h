#pragma once

#include "../CharaBase.h"

class StaffBase :public CharaBase
{
public:
	StaffBase() { Init(); };
	~StaffBase() {};

	virtual void Init() override ;
	virtual void Update() override ;

	void SetTarget(const std::weak_ptr<KdGameObject>& target)
	{ 
		m_wpTarget = target; 
	}

	void SetAngle(float angle)
	{
		m_angle = angle;
	}

protected:

	std::weak_ptr<KdGameObject> m_wpTarget;

	float MagicCD;

private:

	float m_angle = 0.0f;
	float m_radius = 1.5f;
	float m_height = 2.0f;
	float m_rotateSpeed = 0.03f;

};