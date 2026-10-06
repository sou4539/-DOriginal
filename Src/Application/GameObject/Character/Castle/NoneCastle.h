#pragma once

#include "../../Character/CharaBase.h"

// 草原に置く城の外装。ボスマップ用のCastleとは別のオブジェクト。
class NoneCastle : public CharaBase
{
public:
	explicit NoneCastle(const Math::Vector3& villageCenter);
	void GenerateDepthMapFromLight() override;
};
