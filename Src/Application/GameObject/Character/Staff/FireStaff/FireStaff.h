#pragma once

#include "../StaffBase.h"

class FireStaff :public StaffBase
{
public:
	FireStaff() { Init(); }
	~FireStaff() override {}
	void Init() override;
	void Update() override;

	

private:
};