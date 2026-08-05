#pragma once

#include "../StaffBase.h"

class IceStaff :public StaffBase
{
public:
	IceStaff() { Init(); }
	~IceStaff() override {}
	void Init() override;
	void Update() override;



private:
};