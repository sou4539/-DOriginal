#include "BatGroup.h"

#include "Bat.h"

// BatGroupの初期化処理。
// 使い方：
//   BatGroup生成時にコンストラクタから自動で呼ばれる。
// 処理内容：
//   今はまだ群れ自体が保持する情報がないため、処理は空。
//   今後、群れの中心座標や出現範囲を持たせるならここで初期化する。
void BatGroup::Init()
{
}

// BatGroupの更新処理。
// 使い方：
//   シーンのUpdate処理から毎フレーム自動で呼ばれる。
// 処理内容：
//   今は個別のBatが自分で移動・追跡を行うため、ここでは処理しない。
//   将来的に群れ全体の出現・消滅・一括制御をしたい場合に使う。
void BatGroup::Update()
{
}

// 群れ全体のターゲット設定。
// 使い方：
//   本来はBatGroupにターゲットを渡し、生成するBat全体に共有する時に使う。
// 現在：
//   AddBatsToScene()で各Batへ直接targetを渡しているため、この関数は未使用。
void BatGroup::SetTarget(const std::shared_ptr<KdGameObject>& target)
{
	(void)target;
}

// コウモリを複数生成してシーンへ追加する処理。
// 使い方：
//   GameScene::Init()で、batGroup->AddBatsToScene(m_objList, player) の形で呼ぶ。
// 引数：
//   objList : 作成したBatを追加するシーンのオブジェクトリスト。
//   target  : Batが追いかける対象。今はPlayerを渡している。
// 処理内容：
//   batStartPositionsに書いた座標ごとにBatを作り、初期位置とターゲットを設定する。
void BatGroup::AddBatsToScene(std::list<std::shared_ptr<KdGameObject>>& objList, const std::shared_ptr<KdGameObject>& target)
{
	// コウモリの初期位置をここにまとめる。
	// 数を増やしたい時は、この配列に座標を追加するだけでよい。
	const Math::Vector3 batStartPositions[] =
	{
		{ -15.0f, 3.0f,  0.0f },
		{ -20.0f, 3.0f,  5.0f },
		{ -20.0f, 3.0f, -5.0f },
		{ -25.0f, 3.0f,  0.0f },
	};

	for (const Math::Vector3& startPos : batStartPositions)
	{
		std::shared_ptr<Bat> bat = std::make_shared<Bat>();
		bat->SetStartPos(startPos);
		bat->SetTarget(target);
		objList.push_back(bat);
	}
}
