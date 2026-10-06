#pragma once

#include "../StageBase.h"

// モデル確認用の城。移動を妨げず確認できるよう、当たり判定はまだ付けない。
class Castle : public StageBase
{
public:
	Castle() { Init(); }
	~Castle() override = default;
	void GenerateDepthMapFromLight() override;

private:
	void Init() override;
};
