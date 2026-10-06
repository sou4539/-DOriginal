#pragma once

#include "../GameStageBase.h"

class GrassStage : public GameStageBase
{
public:

	// Statusを受け取った後、Startから初期化する。
	GrassStage() = default;
	~GrassStage() override{}

private:
	void Init() override;

};
