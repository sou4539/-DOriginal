#include "Status.h"

#include "../../../main.h"

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

	// EXPバーはHPバーの下に小さく表示する。
	constexpr int ExpBarX = HpBarX + 10;
	constexpr int ExpBarY = HpBarY - 52;
	constexpr int ExpBarDrawW = 300;
	constexpr int ExpBarDrawH = 28;
	constexpr int ExpGaugeOffsetX = 32;
	constexpr int ExpGaugeOffsetY = 7;
	constexpr int ExpGaugeW = 250;
	constexpr int ExpGaugeH = 14;
	const Math::Color ExpBarTintColor = { 0.35f, 0.55f, 1.0f, 1.0f };
	const Math::Color ExpGaugeColor = { 0.05f, 0.35f, 1.0f, 1.0f };
	const Math::Color ExpHideColor = HpHideColor;

	// Number.pngは0～9が横一列に並んでいる。
	constexpr int NumberSrcW = 50;
	constexpr int NumberSrcH = 100;
	constexpr int LevelNumberX = ExpBarX + 5;
	constexpr int LevelNumberY = ExpBarY - 34;
	constexpr int NumberDrawW = 18;
	constexpr int NumberDrawH = 36;

	// レベルアップ選択UIの表示設定。
	// 新しいBack.pngは横長寄りなので、画像比率に近いカードサイズで3枚並べる。
	// 文字画像もカードに対して大きめに描画し、上下左右の余白が出すぎないようにする。
	constexpr int LevelUpCardY = 10;
	constexpr int LevelUpCardW = 300;
	constexpr int LevelUpCardH = 220;
	constexpr int LevelUpIconY = 0;
	constexpr int LevelUpIconSize = 150;
	constexpr int LevelUpCardXList[3] = { -340, 0, 340 };

	bool IsMouseInSprite(const POINT& mousePos, int centerX, int centerY, int width, int height)
	{
		const int spriteX = mousePos.x - 640;
		const int spriteY = 360 - mousePos.y;

		const int halfW = width / 2;
		const int halfH = height / 2;

		return spriteX >= centerX - halfW &&
			   spriteX <= centerX + halfW &&
			   spriteY >= centerY - halfH &&
			   spriteY <= centerY + halfH;
	}
}

// Statusの初期化処理。
// 使い方：
//   Status生成時にコンストラクタから自動で呼ばれる。
//   HPバー画像を差し替えた場合は、Load()のパスを変更する。
// 処理内容：
//   HPバー画像を読み込み、読み込み失敗時はDrawSpriteで使わないようnullptrに戻す。
void Status::Init()
{
	auto loadTexture = [](const char* path)
	{
		std::shared_ptr<KdTexture> tex = std::make_shared<KdTexture>();
		if (!tex->Load(path))
		{
			return std::shared_ptr<KdTexture>(nullptr);
		}
		return tex;
	};

	// HPバーに使う画像を読み込む。
	// 画像はAsset/Textures/UIフォルダに置いている横向きバーを使う。
	if (!m_hpBarTex)
	{
		// 現在の素材配置に合わせて、LevelUpフォルダ内のHP_Bar.pngを読む。
		// Loadに失敗した場合はnullptrにして、DrawSpriteで描画しないようにする。
		m_hpBarTex = loadTexture("Asset/Textures/UI/HP_Bar.png");
	}

	// レベルアップ選択UI用の画像を読み込む。
	// まだ選択処理は入れず、まずは「Backの上に文字画像を重ねる」表示だけ作る。
	if (!m_levelUpBackTex)
	{
		m_levelUpBackTex = loadTexture("Asset/Textures/UI/LevelUp/Back.png");
	}
	if (!m_fireUpTex)
	{
		m_fireUpTex = loadTexture("Asset/Textures/UI/LevelUp/FireUp.png");
	}
	if (!m_iceUpTex)
	{
		m_iceUpTex = loadTexture("Asset/Textures/UI/LevelUp/IceUp.png");
	}
	if (!m_voltUpTex)
	{
		m_voltUpTex = loadTexture("Asset/Textures/UI/LevelUp/VoltUp.png");
	}
	if (!m_numberTex)
	{
		m_numberTex = loadTexture("Asset/Textures/UI/LevelUp/Number.png");
	}
}

void Status::Update()
{
	if (m_isLevelUpSelect)
	{
		UpdateLevelUpSelect();
	}

	// デバッグ用：Qキーを押した瞬間にレベルアップさせる。
	// 押しっぱなしで毎フレーム上がらないように、前フレームの入力状態と比較する。
	const bool isDebugLevelUpKey = (GetAsyncKeyState('Q') & 0x8000);
	if (isDebugLevelUpKey && !m_prevDebugLevelUpKey)
	{
		LevelUp();
	}
	m_prevDebugLevelUpKey = isDebugLevelUpKey;
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
	if (m_isLevelUpSelect)
	{
		DrawLevelUpSelect();
	}

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

	DrawExpBar();
	DrawNumber(m_level, LevelNumberX, LevelNumberY, NumberDrawW, NumberDrawH);
}

void Status::DrawExpBar()
{
	if (!m_hpBarTex) { return; }
	if (!m_hpBarTex->GetSRView()) { return; }

	Math::Rectangle srcRect = HpBarSrcRect;
	KdShaderManager::Instance().m_spriteShader.DrawTex
	(
		m_hpBarTex.get(),
		ExpBarX,
		ExpBarY,
		ExpBarDrawW,
		ExpBarDrawH,
		&srcRect,
		&ExpBarTintColor,
		{ 0.0f, 0.0f }
	);

	float expRate = 0.0f;
	if (m_nextExp > 0.0f)
	{
		expRate = m_exp / m_nextExp;
	}
	expRate = std::clamp(expRate, 0.0f, 1.0f);

	const int innerX = ExpBarX + ExpGaugeOffsetX;
	const int innerY = ExpBarY + ExpGaugeOffsetY;
	const int fillW = static_cast<int>(ExpGaugeW * expRate);
	const int hideW = ExpGaugeW - fillW;

	if (fillW > 0)
	{
		KdShaderManager::Instance().m_spriteShader.DrawBox
		(
			innerX + (fillW / 2),
			innerY + (ExpGaugeH / 2),
			fillW / 2,
			ExpGaugeH / 2,
			&ExpGaugeColor,
			true
		);
	}

	if (hideW > 0)
	{
		KdShaderManager::Instance().m_spriteShader.DrawBox
		(
			innerX + fillW + (hideW / 2),
			innerY + (ExpGaugeH / 2),
			hideW / 2,
			ExpGaugeH / 2,
			&ExpHideColor,
			true
		);
	}
}

void Status::DrawNumber(int value, int x, int y, int drawW, int drawH)
{
	if (!m_numberTex) { return; }
	if (!m_numberTex->GetSRView()) { return; }

	if (value < 0) { value = 0; }

	std::string text = std::to_string(value);
	for (int i = 0; i < static_cast<int>(text.size()); ++i)
	{
		const int number = text[i] - '0';
		if (number < 0 || number > 9) { continue; }

		Math::Rectangle srcRect =
		{
			number * NumberSrcW,
			0,
			NumberSrcW,
			NumberSrcH
		};

		KdShaderManager::Instance().m_spriteShader.DrawTex
		(
			m_numberTex.get(),
			x + (i * drawW),
			y,
			drawW,
			drawH,
			&srcRect,
			&kWhiteColor,
			{ 0.0f, 0.0f }
		);
	}
}

void Status::DrawLevelUpSelect()
{
	if (!m_levelUpBackTex) { return; }
	if (!m_levelUpBackTex->GetSRView()) { return; }

	KdTexture* iconTexList[3] =
	{
		m_fireUpTex.get(),
		m_iceUpTex.get(),
		m_voltUpTex.get()
	};

	for (int i = 0; i < 3; ++i)
	{
		const int cardX = LevelUpCardXList[i];

		// まず背景カードを描画する。
		// pivotを中央にしておくと、3枚のカードを画面中央基準で並べやすい。
		KdShaderManager::Instance().m_spriteShader.DrawTex
		(
			m_levelUpBackTex.get(),
			cardX,
			LevelUpCardY,
			LevelUpCardW,
			LevelUpCardH,
			nullptr,
			&kWhiteColor,
			{ 0.5f, 0.5f }
		);

		// 背景カードの上に、魔法名の画像を重ねる。
		// 画像が読み込めていない場合だけ、そのカードの文字表示を飛ばす。
		KdTexture* iconTex = iconTexList[i];
		if (!iconTex) { continue; }
		if (!iconTex->GetSRView()) { continue; }

		KdShaderManager::Instance().m_spriteShader.DrawTex
		(
			iconTex,
			cardX,
			LevelUpIconY,
			LevelUpIconSize,
			LevelUpIconSize,
			nullptr,
			&kWhiteColor,
			{ 0.5f, 0.5f }
		);
	}
}

void Status::UpdateLevelUpSelect()
{
	POINT mousePos;
	GetCursorPos(&mousePos);
	ScreenToClient(Application::Instance().GetWindowHandle(), &mousePos);

	const bool isLeftClick = (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
	if (!isLeftClick || m_prevLeftClick)
	{
		m_prevLeftClick = isLeftClick;
		return;
	}

	if (IsMouseInSprite(mousePos, LevelUpCardXList[0], LevelUpCardY, LevelUpCardW, LevelUpCardH))
	{
		EnhanceFire();
		m_isLevelUpSelect = false;
	}
	else if (IsMouseInSprite(mousePos, LevelUpCardXList[1], LevelUpCardY, LevelUpCardW, LevelUpCardH))
	{
		EnhanceIce();
		m_isLevelUpSelect = false;
	}
	else if (IsMouseInSprite(mousePos, LevelUpCardXList[2], LevelUpCardY, LevelUpCardW, LevelUpCardH))
	{
		EnhanceVolt();
		m_isLevelUpSelect = false;
	}

	m_prevLeftClick = isLeftClick;
}

void Status::EnhanceFire()
{
	// 炎は範囲魔法として育てる。
	// 選ぶたびに命中時の爆発範囲が広がる。
	m_fireExplosionRadius += 0.5f;
}

void Status::EnhanceIce()
{
	// 氷は「命中後に派生弾が出る」魔法として育てる。
	// 最初から弾数を増やすと画面がうるさくなるため、
	// 選ぶたびに貫通数と、命中後に出る半分ダメージの派生弾数を伸ばす。
	m_iceSplitCount++;
	m_icePierceCount++;
}

void Status::EnhanceVolt()
{
	// 雷は連鎖魔法として育てる。
	// 初期状態でも1回連鎖し、選ぶたびにさらに連鎖数が増える。
	m_voltChainCount++;
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

// 経験値を加算する処理。
// 使い方：
//   敵を倒した時に、敵側から spStatus->AddExp(経験値量) の形で呼ぶ。
// 処理内容：
//   受け取った経験値を現在経験値に足し、必要経験値に届いたらレベルアップする。
// 注意：
//   0以下の値は無視する。
void Status::AddExp(float exp)
{
	if (exp <= 0.0f) { return; }

	m_exp += exp;

	// 一度に大量の経験値を得た場合、複数回レベルアップできるようにwhileで確認する。
	while (m_exp >= m_nextExp)
	{
		m_exp -= m_nextExp;
		LevelUp();
	}
}

// レベルアップ処理。
// 使い方：
//   AddExp()内で必要経験値に到達した時だけ呼ぶ。
// 処理内容：
//   レベルを上げ、最大HPと攻撃力を上げる。
//   レベルアップ時は現在HPも最大値まで回復する。
void Status::LevelUp()
{
	m_level++;

	// レベルアップしたら、魔法強化選択UIを表示する。
	// 現時点では表示確認だけ行い、選択して閉じる処理は次の作業で追加する。
	m_isLevelUpSelect = true;

	// レベルアップによる成長量。
	// まずは分かりやすく、最大HPと攻撃力を固定値で増やす。
	m_pMaxHp += 10.0f;
	m_pAttack += 2.0f;

	// レベルアップした気持ちよさを出すため、HPを全回復する。
	m_pHp = m_pMaxHp;

	// 次レベルの必要経験値を少しずつ増やす。
	// 例：100 → 125 → 156.25 のように増えていく。
	m_nextExp *= 1.25f;
}




















