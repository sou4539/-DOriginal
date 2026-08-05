#include "Status.h"

namespace
{
	// HPバーの表示設定。
	// 画像を差し替えたり、表示位置を調整したい時はまずここを見る。
	constexpr int HpBarX = -660;
	constexpr int HpBarY = 320;
	constexpr int HpBarDrawW = 400;
	constexpr int HpBarDrawH = 45;

	// HP_Bar.png の一番上にある横バー部分。
	const Math::Rectangle HpBarSrcRect = { 0, 0, 384, 128 };

	// 表示後のHPバー内で、実際に赤ゲージが入っている範囲。
	// 減ったHPはこの範囲だけを右から隠す。
	constexpr int HpGaugeOffsetX = 42;
	constexpr int HpGaugeOffsetY = 10;
	constexpr int HpGaugeW = 333;
	constexpr int HpGaugeH = 24;

const Math::Color HpHideColor = { 0.08f, 0.07f, 0.13f, 1.0f };
}

// Statusの初期化処理。
// 使い方：
//   Status生成時にコンストラクタから自動で呼ばれる。
//   HPバー画像を差し替えた場合は、Load()のパスを変更する。
// 処理内容：
//   HPバー画像を読み込み、読み込み失敗時はDrawSpriteで使わないようnullptrに戻す。
void Status::Init()
{
	// HPバーに使う画像を読み込む。
	// 画像はAsset/Textures/UIフォルダに置いている横向きバーを使う。
	if (!m_hpBarTex)
	{
		m_hpBarTex = std::make_shared<KdTexture>();

		// Loadに失敗した場合、m_hpBarTex自体は存在していても中身の画像情報が空になる。
		// その状態でDrawTexすると例外の原因になるため、失敗時はnullptrに戻しておく。
		if (!m_hpBarTex->Load("Asset/Textures/UI/HP_Bar.png"))
		{
			m_hpBarTex = nullptr;
		}
	}
}

void Status::Update()
{
}

// HPバーの描画処理。
// 使い方：
//   BaseSceneのDrawSpriteから毎フレーム自動で呼ばれる。
// 処理内容：
//   満タン状態のHPバー画像を描画し、減ったHP分だけ右側を暗い色で隠す。
// 注意：
//   HPバー画像のサイズやデザインを変えた場合は、上のHpGauge系定数も調整する。
void Status::DrawSprite()
{
	if (!m_hpBarTex) { return; }
	if (!m_hpBarTex->GetSRView()) { return; }

	// 画面サイズは1280x720なので、2D座標はだいたい
	// 左端が -640、右端が 640、上端が 360、下端が -360 になる。
	// HPバーは画面左上に置くため、かなり左寄りの座標を使う。
	const int barX = HpBarX;
	const int barY = HpBarY;

	// 表示するHPバーの大きさ。
	// 元画像は384x384で、横バーが縦に3種類並んでいる。
	// ここでは一番上の横バー部分だけを切り抜いて使う。
	const int drawW = HpBarDrawW;
	const int drawH = HpBarDrawH;

	// 元画像の一番上の横バー部分を切り抜く。
	// 0～128pxの範囲に、満タン状態の横バーが入っている。
	Math::Rectangle srcRect = HpBarSrcRect;

	// HPバー画像を描画する。
	// pivotを左上基準にすることで、barX/barYを左上座標として扱える。
	KdShaderManager::Instance().m_spriteShader.DrawTex
	(
		m_hpBarTex.get(),
		barX,
		barY,
		drawW,
		drawH,
		&srcRect,
		&kWhiteColor,
		{ 0.0f, 0.0f }
	);

	// 現在HPの割合を0.0～1.0に収める。
	float hpRate = 0.0f;
	if (m_pMaxHp > 0.0f)
	{
		hpRate = m_pHp / m_pMaxHp;
	}
	hpRate = std::clamp(hpRate, 0.0f, 1.0f);

	// HP_Bar.pngの赤い部分は、画像の内側に余白がある。
	// そのため、赤バーの内側だけを隠すように座標を少し内側へずらす。
	const int innerX = barX + HpGaugeOffsetX;
	// 赤ゲージの上端が1pxほど残らないように、隠す範囲を少し上から始める。
	const int innerY = barY + HpGaugeOffsetY;
	const int innerW = HpGaugeW;
	const int innerH = HpGaugeH;

	// 減ったHP分だけ、右側を暗い色で隠す。
	// 画像自体は満タンの赤バーなので、足りない部分を上から塗って減って見せる。
	const int hideW = static_cast<int>(innerW * (1.0f - hpRate));
	if (hideW > 0)
	{
		const int hideCenterX = innerX + innerW - (hideW / 2);
		const int hideCenterY = innerY + (innerH / 2);

		KdShaderManager::Instance().m_spriteShader.DrawBox
		(
			hideCenterX,
			hideCenterY,
			hideW / 2,
			innerH / 2,
			&HpHideColor,
			true
		);
	}

	// ここでは文字表示は行わない。
	// DrawFontは内部でフォント画像を生成するため、環境によって例外原因になりやすい。
	// まずはHPバー画像だけでHPの減少を確認できるようにしている。
}

// プレイヤーHPを減らす処理。
// 使い方：
//   敵やダメージ判定側から spStatus->DamagePlayer(ダメージ量) の形で呼ぶ。
// 処理内容：
//   受け取ったダメージ量だけHPを減らし、0未満にならないようにする。
// 注意：
//   0以下の値は無視する。回復したい場合は別の回復用関数を使う。
void Status::DamagePlayer(float damage)
{
	// 0以下の値を受け取った場合は何もしない。
	// 回復処理はResetPlayerHpなど別関数に分け、DamagePlayerは「減らすだけ」にする。
	if (damage <= 0.0f) { return; }

	// 受け取ったダメージ量だけ、プレイヤーHPを減らす。
	m_pHp -= damage;

	// HPが0より下に行くと、UI表示や死亡判定が扱いにくくなる。
	// そのため、最低値は0で止めておく。
	if (m_pHp < 0.0f)
	{
		m_pHp = 0.0f;
	}
}

// プレイヤーHPを最大まで戻す処理。
// 使い方：
//   Playerの復活処理から呼ぶ。
// 処理内容：
//   現在HPを最大HPと同じ値に戻し、HPバー表示も満タンにする。
void Status::ResetPlayerHp()
{
	// 村で復活する時は、プレイヤーHPを最大値まで戻す。
	// HPバーはm_pHp / m_pMaxHpで表示しているため、ここを戻すだけで表示も満タンになる。
	m_pHp = m_pMaxHp;
}
