#pragma once

#include "../StageBase.h"

class Ground : public StageBase
{
public:
	Ground() { Init(); }
	~Ground() override {}

private:
	void Init() override;
};
