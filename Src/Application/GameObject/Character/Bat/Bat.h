#pragma once

#include "../CharaBase.h"

class Bat : public CharaBase
{
public:
	Bat() { Init(); }
	~Bat() override {}

	void Init() override;
	void Update() override;

private:
	/*
		Bat.gltfに入っているアニメーションを再生・更新するためのクラス。
		今回は羽ばたき用の "flap_loop" を再生する。
	*/
	KdAnimator m_animator;
};
