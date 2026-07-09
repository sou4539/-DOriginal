#pragma once
#include "../CharaBase.h"

class Status;

class Player : public CharaBase
{
public:
	Player() { Init(); }
	~Player()				override	{}

	
	void Update()			override;
	void PostUpdate()		override;
	//void DrawLit()			override;

	void SetStatus(const std::shared_ptr<Status>& status) 
	{ 
		m_status = status;
	}
private:
	std::weak_ptr<Status> m_status;

	void Init()				override;
};