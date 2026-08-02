#pragma once

#include "../CharaBase.h"

class Bat;

class BatGroup : public CharaBase
{
public:
	BatGroup() { Init(); }
	~BatGroup() override {}

	void Init() override;
	void Update() override;

	void SetTarget(const std::shared_ptr<KdGameObject>& target);
	void AddBatsToScene(std::list<std::shared_ptr<KdGameObject>>& objList, const std::shared_ptr<KdGameObject>& target);
};
