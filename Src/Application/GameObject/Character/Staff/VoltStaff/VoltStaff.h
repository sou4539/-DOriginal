#pragma once

#include "../StaffBase.h"

class VoltStaff :public StaffBase
{
public:
	VoltStaff() { Init(); }
	~VoltStaff() override {}
	void Init() override;
	void Update() override;



private:
};