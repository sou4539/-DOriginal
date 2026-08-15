#include "BatGroup.h"

#include "Bat.h"
#include "../Player/Player.h"

#include <random>

// BatGroupの初期化処理。
// 使い方：
//   BatGroup生成時にコンストラクタから自動で呼ばれる。
// 処理内容：
//   コウモリの生息地を確認するためのデバッグワイヤを作成する。
void BatGroup::Init()
{
	m_pDebugWire = std::make_unique<KdDebugWireFrame>();
}

// BatGroupの更新処理。
// 使い方：
//   シーンのUpdate処理から毎フレーム自動で呼ばれる。
// 処理内容：
//   コウモリの生息地をデバッグ表示する。
//   さらに、プレイヤーが村の安全地帯に入った瞬間だけ死んだコウモリを補充する。
void BatGroup::Update()
{
	// コウモリの生息地をデバッグ表示する。
	// この白いスフィア内のランダムな位置にコウモリを生成・補充する。
	if (m_pDebugWire)
	{
		for (const Habitat& habitat : m_habitats)
		{
			m_pDebugWire->AddDebugSphere(habitat.center, habitat.radius, kWhiteColor);
		}
	}

	std::shared_ptr<Player> spPlayer = std::dynamic_pointer_cast<Player>(m_wpTarget.lock());
	if (!spPlayer) { return; }

	// 村の安全地帯に入った瞬間だけ、死んだコウモリを補充する。
	// 安全地帯内にいる間ずっと補充判定すると毎フレーム処理されるため、
	// 前フレームの状態と比較して「外から中へ入った時」だけ処理する。
	bool isTargetInSafeArea = spPlayer->IsInSafeArea();
	if (isTargetInSafeArea && !m_wasTargetInSafeArea)
	{
		RefillDeadBats();
	}

	m_wasTargetInSafeArea = isTargetInSafeArea;
}

// 群れ全体のターゲット設定。
// 使い方：
//   BatGroupが生成したBat全体に、同じ追跡対象を共有する時に使う。
//   また、Playerが安全地帯に入った瞬間を確認するためにも使う。
void BatGroup::SetTarget(const std::shared_ptr<KdGameObject>& target)
{
	m_wpTarget = target;
}

// コウモリの生息地追加。
// 使い方：
//   GameScene::Init()で、batGroup->AddHabitat(中心座標, 半径, コウモリ数) の形で呼ぶ。
// 処理内容：
//   コウモリが出現する白いスフィア範囲を追加する。
//   生息地を複数追加すると、それぞれの範囲に指定数のコウモリが出現する。
void BatGroup::AddHabitat(const Math::Vector3& center, float radius, int count)
{
	Habitat habitat;
	habitat.center = center;
	habitat.radius = radius;
	habitat.count = count;

	m_habitats.push_back(habitat);
}

// コウモリを複数生成してシーンへ追加する処理。
// 使い方：
//   GameScene::Init()で、batGroup->AddBatsToScene(m_objList, player) の形で呼ぶ。
// 引数：
//   objList : 作成したBatを追加するシーンのオブジェクトリスト。
//   target  : Batが追いかける対象。今はPlayerを渡している。
// 処理内容：
//   登録されている生息地ごとに、指定数のBatをランダム配置する。
void BatGroup::AddBatsToScene(std::list<std::shared_ptr<KdGameObject>>& objList, const std::shared_ptr<KdGameObject>& target)
{
	SetTarget(target);
	m_pObjList = &objList;
	m_bats.clear();

	for (int habitatIndex = 0; habitatIndex < static_cast<int>(m_habitats.size()); ++habitatIndex)
	{
		const Habitat& habitat = m_habitats[habitatIndex];

		for (int i = 0; i < habitat.count; ++i)
		{
			AddBatToScene(objList, target, habitatIndex);
		}
	}
}

// コウモリのランダム座標作成。
// 使い方：
//   初回生成時と補充時の両方から呼ぶ。
// 処理内容：
//   指定された生息地の中心から見たランダムな方向と距離を作り、
//   XZ平面上の生息地範囲内にコウモリを配置する。
//   Y座標は生息地の中心と同じ高さに固定する。
Math::Vector3 BatGroup::MakeRandomPos(int habitatIndex) const
{
	if (habitatIndex < 0 || habitatIndex >= static_cast<int>(m_habitats.size()))
	{
		return Math::Vector3::Zero;
	}

	const Habitat& habitat = m_habitats[habitatIndex];

	static std::mt19937 randomEngine(std::random_device{}());
	std::uniform_real_distribution<float> angleDist(0.0f, DirectX::XM_2PI);
	std::uniform_real_distribution<float> radiusDist(0.0f, 1.0f);

	float angle = angleDist(randomEngine);
	float radius = std::sqrt(radiusDist(randomEngine)) * habitat.radius;

	Math::Vector3 pos;
	pos.x = habitat.center.x + std::cos(angle) * radius;
	pos.y = habitat.center.y;
	pos.z = habitat.center.z + std::sin(angle) * radius;

	return pos;
}

// コウモリの補充処理。
// 使い方：
//   プレイヤーが村の安全地帯に入った瞬間に呼ぶ。
// 処理内容：
//   生きているコウモリはそのまま残し、死んでいるコウモリだけ管理リストから外す。
//   その後、生息地ごとの数が設定数に戻るように足りない分だけ新しく生成する。
void BatGroup::RefillDeadBats()
{
	std::vector<BatInfo> liveBats;

	for (BatInfo& batInfo : m_bats)
	{
		std::shared_ptr<Bat> spBat = batInfo.bat.lock();
		if (!spBat) { continue; }
		if (spBat->IsExpired()) { continue; }

		// 生きているコウモリは補充対象ではない。
		// ここでSetStartPos()を呼ぶと、戦闘中のコウモリまで一括リセットされてしまう。
		liveBats.push_back(batInfo);
	}

	m_bats = liveBats;

	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
	if (!m_pObjList || !spTarget) { return; }

	// 倒されて消えたコウモリがいる場合は、
	// 生息地ごとの数が設定数に戻るように補充する。
	for (int habitatIndex = 0; habitatIndex < static_cast<int>(m_habitats.size()); ++habitatIndex)
	{
		int batCountInHabitat = 0;
		for (const BatInfo& batInfo : m_bats)
		{
			if (batInfo.habitatIndex == habitatIndex)
			{
				++batCountInHabitat;
			}
		}

		while (batCountInHabitat < m_habitats[habitatIndex].count)
		{
			AddBatToScene(*m_pObjList, spTarget, habitatIndex);
			++batCountInHabitat;
		}
	}
}

// コウモリの追加処理。
// 使い方：
//   初回生成時と、村に入った時の補充処理から呼ぶ。
// 処理内容：
//   指定した生息地の範囲内にランダムな初期位置を作り、
//   Batに開始位置と追跡対象を設定してシーンへ追加する。
void BatGroup::AddBatToScene(std::list<std::shared_ptr<KdGameObject>>& objList, const std::shared_ptr<KdGameObject>& target, int habitatIndex)
{
	Math::Vector3 startPos = MakeRandomPos(habitatIndex);

	std::shared_ptr<Bat> bat = std::make_shared<Bat>();
	bat->SetStartPos(startPos);
	bat->SetTarget(target);
	objList.push_back(bat);

	BatInfo batInfo;
	batInfo.bat = bat;
	batInfo.habitatIndex = habitatIndex;
	m_bats.push_back(batInfo);
}









