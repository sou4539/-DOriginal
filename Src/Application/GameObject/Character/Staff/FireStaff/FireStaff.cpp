#include "FireStaff.h"

#include "../Magic/MagicBase.h"
#include "../../../../Scene/SceneManager.h"
#include "../../Bat/Bat.h"

void FireStaff::Init()
{
	m_spModel = std::make_shared<KdModelWork>();
	m_spModel->SetModelData("Asset/Models/Objects/Character/Staff/FireStaff.gltf");

	MagicCD = 0.0f;

	
}

void FireStaff::Update()
{
	// 杖をプレイヤーの周りで回転させる
	StaffBase::Update();

	// 杖の追従対象、つまりプレイヤーを取得する
	auto spPlayer = m_wpTarget.lock();
	if (!spPlayer)
	{
		return;
	}

	// 魔法のクールタイムを減らす
	MagicCD--;

	// クールタイムが残っているなら、まだ撃たない
	if (MagicCD > 0.0f)
	{
		return;
	}

	// プレイヤーを中心に、この範囲内の敵を探す
	float searchRadius = 8.0f;

	// 今見つかっている一番近い敵
	std::shared_ptr<KdGameObject> spTargetEnemy = nullptr;

	// 一番近い距離を保存する
	// 最初は索敵範囲の最大距離を入れておく
	float minDistanceSqr = searchRadius * searchRadius;

	// シーン内の全オブジェクトを調べる
	for (auto& spObj : SceneManager::Instance().GetObjList())
	{
		// Bat以外は無視する
		auto spBat = std::dynamic_pointer_cast<Bat>(spObj);
		if (!spBat)
		{
			continue;
		}

		// プレイヤーからBatまでの方向と距離を調べる
		Math::Vector3 toEnemy = spBat->GetPos() - spPlayer->GetPos();
		float distanceSqr = toEnemy.LengthSquared();

		// 今まで見つけた敵より近ければ、このBatを狙う
		if (distanceSqr < minDistanceSqr)
		{
			minDistanceSqr = distanceSqr;
			spTargetEnemy = spBat;
		}
	}

	// 範囲内に敵がいなければ撃たない
	if (!spTargetEnemy)
	{
		return;
	}

	// 杖から敵へ向かう方向を作る
	Math::Vector3 shotDir = spTargetEnemy->GetPos() - GetPos();
	shotDir.Normalize();

	// 魔法を作って、敵の方向へ飛ばす
	std::shared_ptr<MagicBase> magic = std::make_shared<MagicBase>();
	magic->Shot(GetPos(), shotDir, MagicType::Fire, 10.0f, 0.2f);
	SceneManager::Instance().AddObject(magic);

	// 1秒のクールタイム
	MagicCD = 60.0f;
}
