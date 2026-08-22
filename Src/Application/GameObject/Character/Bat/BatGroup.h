#pragma once

#include "../CharaBase.h"
#include <vector>

class Bat;
class Status;

class BatGroup : public CharaBase
{
public:
	// 生成時に初期化も行う。
	BatGroup() { Init(); }

	~BatGroup() override {}

	// デバッグ表示の準備をする。
	void Init() override;

	// 出現範囲表示とコウモリ数の維持を行う。
	void Update() override;

	// 生息地を1つだけ設定する。
	void SetGroupParam(const Math::Vector3& center, float radius, int count)
	{
		m_habitats.clear();
		AddHabitat(center, radius, count);
	}

	// コウモリの生息地を追加する。
	void AddHabitat(const Math::Vector3& center, float radius, int count);

	// 群れ全体の追跡対象を設定する。
	void SetTarget(const std::shared_ptr<KdGameObject>& target);

	// 撃破時に経験値を渡すStatusを設定する。
	void SetStatus(const std::shared_ptr<Status>& status)
	{
		m_wpStatus = status;
	}

	// コウモリを出さない安全地帯を設定する。
	void SetSafeArea(const Math::Vector3& center, float radius)
	{
		m_safeAreaCenter = center;
		m_safeAreaRadius = radius;
	}

	// 初期コウモリをシーンへ追加する。
	void AddBatsToScene(std::list<std::shared_ptr<KdGameObject>>& objList, const std::shared_ptr<KdGameObject>& target);


	void UpdateBatCountByDistance();
	int GetTotalBatCount() const
	{
		return static_cast<int>(m_bats.size());
	}


private:
	// 生息地ごとの設定。
	struct Habitat
	{
		Math::Vector3 center = Math::Vector3::Zero;
		float radius = 0.0f;
		int count = 0;
	};

	// 生成したコウモリと所属生息地。
	struct BatInfo
	{
		std::weak_ptr<Bat> bat;
		int habitatIndex = 0;
	};

	// プレイヤー周辺のランダム出現座標を作る。
	Math::Vector3 MakeRandomPos(int habitatIndex) const;

	// 指定座標が安全地帯内か確認する。
	bool IsInSafeArea(const Math::Vector3& pos) const;

	// 指定座標が出現スフィア外か確認する。
	bool IsOutsideSpawnSphere(const Math::Vector3& pos) const;

	// コウモリ数を設定数に保つ。
	void MaintainBatCount();

	// コウモリを1体追加する。
	void AddBatToScene(std::list<std::shared_ptr<KdGameObject>>& objList, const std::shared_ptr<KdGameObject>& target, int habitatIndex);

	// 登録済みの生息地。
	std::vector<Habitat> m_habitats;

	// このBatGroupが生成したコウモリ。
	std::vector<BatInfo> m_bats;

	// コウモリの追加先リスト。
	std::list<std::shared_ptr<KdGameObject>>* m_pObjList = nullptr;

	// コウモリが追いかける対象。
	std::weak_ptr<KdGameObject> m_wpTarget;

	// 経験値を加算する対象。
	std::weak_ptr<Status> m_wpStatus;

	// プレイヤー基準の出現距離。
	float m_spawnMinRadius = 55.0f;
	float m_spawnMaxRadius = 70.0f;

	// コウモリを出さない安全地帯。
	Math::Vector3 m_safeAreaCenter = Math::Vector3::Zero;
	float m_safeAreaRadius = 0.0f;

	// trueなら補充は1フレーム1体まで。
	bool m_isSpawnOnePerFrame = true;

	//　現在のコウモリの召喚数
	int m_nowMaxBatCount = 20;

	//　最小のコウモリの召喚数
	int m_minBatCount = 20;

	//　最大のコウモリの召喚数
	int m_maxBatCount = 100;

	// コウモリが最大になる距離
	float m_maxBatCountDistance = 250.0f;
};
















