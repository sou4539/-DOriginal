#pragma once

#include "../StageBase.h"

class Village :public StageBase
{
public:
	Village() { Init(); }
	~Village() override {}

	// 村のCOLは壁・柵としても使うため、スフィア判定対象にする。
	bool EnableSphereCollision() const override { return true; }

private:
	void Init() override;

};
