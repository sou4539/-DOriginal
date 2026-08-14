#pragma once

#include "../../CharaBase.h"

enum class MagicType
{
	Fire,
	Ice,
	Volt,
	None
};

class MagicBase:public CharaBase
{
public:

	MagicBase() { Init(); };
	~MagicBase() {};

	void Init();
	void Update();
	void PostUpdate();
	void DrawLit();

	//魔法を発射する関数
	void Shot(const Math::Vector3& startPos, const Math::Vector3& dir, MagicType type, float damage, float speed);
	

private:

	MagicType m_magicType;

	float m_damage;
	float m_speed;
	float m_lifeTime;
	float m_radius;
};
