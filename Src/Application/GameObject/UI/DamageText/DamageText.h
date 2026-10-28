#pragma once

#include "../../Character/Magic/MagicType.h"

class CameraBase;

class DamageText : public KdGameObject
{
public:
	DamageText() {}
	~DamageText() override {}

	void Init() override;
	void Update() override;
	void DrawSprite() override;

	float GetValue() const 
	{ 
		return m_value; 
	}

	void SetValue(float _value)
	{ 
		m_value = _value; 
	}

	void SetType(MagicType _type);

	void SetPos(Math::Vector3 _pos)
	{
		m_pos = _pos;
	}

private:
	MagicType m_type;

	Math::Color m_color;

	std::shared_ptr<KdTexture> m_numberTex;
	float m_value;

	Math::Vector3 m_pos;
	Math::Vector3 m_vec;

	// 表示してから経過したフレーム数
	int m_ViewCount;
};
