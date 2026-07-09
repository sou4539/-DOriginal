#pragma once
#include "../CharaBase.h"

class Player : public CharaBase
{
public:
	Player()							{}
	~Player()				override	{}

	void Init()				override;
	void Update()			override;
	void PostUpdate()		override;
	//void DrawLit()			override;

private:

};