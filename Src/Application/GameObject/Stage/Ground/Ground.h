#pragma once

#include "../StageBase.h"

class Ground : public StageBase
{
public:
	Ground() { Init(); }
	~Ground() override {}

	void DrawLit() override;

private:
	void Init() override;

	// 地面の表示サイズ。
	// 値を大きくすると地面自体が広くなる。
	float m_groundScale = 300.0f;

	// 地面テクスチャの繰り返し回数。
	// 地面を広げた分だけUVも増やすことで、画像が引き延ばされるのを防ぐ。
	Math::Vector2 m_uvTiling = { 3.0f, 3.0f };
};

