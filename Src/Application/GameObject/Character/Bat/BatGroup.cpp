#include "BatGroup.h"

#include "Bat.h"

void BatGroup::Init()
{
}

void BatGroup::Update()
{
}

void BatGroup::SetTarget(const std::shared_ptr<KdGameObject>& target)
{
	(void)target;
}

void BatGroup::AddBatsToScene(std::list<std::shared_ptr<KdGameObject>>& objList, const std::shared_ptr<KdGameObject>& target)
{
	std::shared_ptr<Bat> bat1 = std::make_shared<Bat>();
	bat1->SetStartPos(Math::Vector3(-15.0f, 3.0f, 0.0f));
	bat1->SetTarget(target);
	objList.push_back(bat1);

	std::shared_ptr<Bat> bat2 = std::make_shared<Bat>();
	bat2->SetStartPos(Math::Vector3(-20.0f, 3.0f, 5.0f));
	bat2->SetTarget(target);
	objList.push_back(bat2);

	std::shared_ptr<Bat> bat3 = std::make_shared<Bat>();
	bat3->SetStartPos(Math::Vector3(-20.0f, 3.0f, -5.0f));
	bat3->SetTarget(target);
	objList.push_back(bat3);

	std::shared_ptr<Bat> bat4 = std::make_shared<Bat>();
	bat4->SetStartPos(Math::Vector3(-25.0f, 3.0f, 0.0f));
	bat4->SetTarget(target);
	objList.push_back(bat4);
}
