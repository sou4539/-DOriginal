#pragma once

#include "../StageBase.h"

class Village :public StageBase
{
public:
	Village() { Init(); }
	~Village(){}

	
	void Update() override;
	void PostUpdate() override;

private:
	void Init() override;

};
