#pragma once

#include "../CharaBase.h"
#include <vector>

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

	// コウモリの生息地を1つだけ設定する。
	// 既存の呼び方を残すための関数。
	// 複数の生息地を使う場合は AddHabitat() を使う。
	void SetGroupParam(const Math::Vector3& center, float radius, int count)
	{
		m_habitats.clear();
		AddHabitat(center, radius, count);
	}

	// コウモリの生息地を追加する。
	// center : 生息地の中心座標
	// radius : この半径内にコウモリをランダム配置する
	// count  : この生息地に出現するコウモリの数
	void AddHabitat(const Math::Vector3& center, float radius, int count);

	// 群れ全体の追跡対象を設定するための関数。
	// Playerの安全地帯フラグを確認し、村に入った瞬間の補充判定にも使う。
	void SetTarget(const std::shared_ptr<KdGameObject>& target);

	// Batを複数作成して、シーンのオブジェクトリストに追加する。
	// GameSceneでBatGroupを作った後に呼ぶ。
	void AddBatsToScene(std::list<std::shared_ptr<KdGameObject>>& objList, const std::shared_ptr<KdGameObject>& target);

private:
	// コウモリの生息地情報。
	struct Habitat
	{
		Math::Vector3 center = Math::Vector3::Zero;
		float radius = 0.0f;
		int count = 0;
	};

	// BatGroupが生成したコウモリの情報。
	// どの生息地から出現したかを覚えておくことで、
	// 死んだコウモリを補充する時も、元の生息地内に出現させることができる。
	struct BatInfo
	{
		std::weak_ptr<Bat> bat;
		int habitatIndex = 0;
	};

	// 指定した生息地の範囲内から、コウモリのランダムな配置座標を作る。
	Math::Vector3 MakeRandomPos(int habitatIndex) const;

	// 死んでいる、または削除されたコウモリだけを補充する。
	// 生きているコウモリの位置は変更しない。
	void RefillDeadBats();

	// コウモリを1体作成して、シーンと管理リストに追加する。
	void AddBatToScene(std::list<std::shared_ptr<KdGameObject>>& objList, const std::shared_ptr<KdGameObject>& target, int habitatIndex);

	// 登録されているコウモリの生息地。
	std::vector<Habitat> m_habitats;

	// このBatGroupが生成したコウモリ。
	// 実体の管理はシーン側が行うため、ここではweak_ptrで参照だけ保持する。
	std::vector<BatInfo> m_bats;

	// コウモリを補充する時に追加先として使うシーンのオブジェクトリスト。
	std::list<std::shared_ptr<KdGameObject>>* m_pObjList = nullptr;

	// コウモリが追いかける対象。
	// Playerなら安全地帯に入ったかどうかも確認する。
	std::weak_ptr<KdGameObject> m_wpTarget;

	// 前フレームにターゲットが安全地帯内にいたかどうか。
	// falseからtrueに変わった瞬間だけ、死んだコウモリを補充する。
	bool m_wasTargetInSafeArea = false;
};










