#pragma once

#include "../GameStageBase.h"

class CastleStage : public GameStageBase
{
public:

	// Statusを受け取った後、Startから初期化する。
	CastleStage() = default;
	~CastleStage() override{}

private:
	void Init() override;

};
