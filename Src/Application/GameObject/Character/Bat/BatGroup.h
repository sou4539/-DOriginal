#pragma once

#include "../CharaBase.h"

class Bat;

class BatGroup : public CharaBase
{
public:
	// BatGroupを作成した時に、自動でInit()を呼ぶ。
	// GameSceneでは std::make_shared<BatGroup>() するだけで準備される。
	BatGroup() { Init(); }

	// BatGroup破棄時の処理。
	// 生成したBatはシーンのobjList側で管理されるため、ここでは追加処理を持たせていない。
	~BatGroup() override {}

	// BatGroupの初期化処理。
	// 今は処理なし。今後、群れの中心座標や生成数などを持たせる時に使う。
	void Init() override;

	// BatGroupの毎フレーム更新。
	// 今は処理なし。今後、群れ全体の制御を入れる時に使う。
	void Update() override;

	// 群れ全体の追跡対象を設定するための関数。
	// 今は個別Batへ直接targetを渡しているため未使用。
	void SetTarget(const std::shared_ptr<KdGameObject>& target);

	// Batを複数作成して、シーンのオブジェクトリストに追加する。
	// GameSceneでBatGroupを作った後に呼ぶ。
	void AddBatsToScene(std::list<std::shared_ptr<KdGameObject>>& objList, const std::shared_ptr<KdGameObject>& target);
};
