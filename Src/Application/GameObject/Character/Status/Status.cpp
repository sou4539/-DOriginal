#include "Status.h"

#include "../../../main.h"
#include "../../Camera/CameraBase.h"
#include "../Magic/MagicBase.h"

#include <filesystem>
#include <fstream>

namespace
{
	const char* ProgressSavePath = "Save/Progress.txt";

	// HPバーの表示設定。
	constexpr int HpBarX = -660;
	constexpr int HpBarY = 320;
	constexpr int HpBarDrawW = 400;
	constexpr int HpBarDrawH = 45;

	// HP_Bar.png の一番上にある横バー部分。
	const Math::Rectangle HpBarSrcRect = { 0, 0, 384, 128 };

	// 表示後のHPバー内で、実際に赤ゲージが入っている範囲。
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

}

// Statusの初期化処理。
void Status::Init()
{
	// HPバーに使う画像を読み込む。
	if (!m_hpBarTex)
	{
		m_hpBarTex = LoadTexture("Asset/Textures/UI/HP_Bar.png");
	}

	// レベルアップ選択UI用の画像を読み込む。
	if (!m_levelUpBackTex)
	{
		m_levelUpBackTex = LoadTexture("Asset/Textures/UI/LevelUp/Back.png");
	}
	if (!m_fireUpTex)
	{
		m_fireUpTex = LoadTexture("Asset/Textures/UI/LevelUp/FireUp.png");
	}
	if (!m_iceUpTex)
	{
		m_iceUpTex = LoadTexture("Asset/Textures/UI/LevelUp/IceUp.png");
	}
	if (!m_voltUpTex)
	{
		m_voltUpTex = LoadTexture("Asset/Textures/UI/LevelUp/VoltUp.png");
	}
	if (!m_fireGetTex)
	{
		m_fireGetTex = LoadTexture("Asset/Textures/UI/LevelUp/FireGet.png");
	}
	if (!m_iceGetTex)
	{
		m_iceGetTex = LoadTexture("Asset/Textures/UI/LevelUp/IceGet.png");
	}
	if (!m_voltGetTex)
	{
		m_voltGetTex = LoadTexture("Asset/Textures/UI/LevelUp/VoltGet.png");
	}
	if (!m_numberTex)
	{
		m_numberTex = LoadTexture("Asset/Textures/UI/LevelUp/Number.png");
	}
	if (!m_villageArrowTex)
	{
		m_villageArrowTex = LoadTexture("Asset/Textures/UI/arrow.png");
	}
	if (!m_cursorTex)
	{
		m_cursorTex = LoadTexture("Asset/Textures/UI/Cursor.png");
	}

	LoadProgress();

	// 初回開始時は魔法を持っていないため、最初の魔法取得UIを開く。
	if (!m_stats.HasAnyMagic())
	{
		m_pendingChoices = 1;
		OpenLevelUpSelect();
	}
}

void Status::Update()
{
	if (m_isChoosingMagic)
	{
		UpdateLevelUpSelect();
	}

	UpdateDebugKeys();
}

void Status::UpdateDebugKeys()
{
	// デバッグ用：Qキーを押した瞬間にレベルアップさせる。
	const bool levelUpDown = (GetAsyncKeyState('Q') & 0x8000);
	if (levelUpDown && !m_prevLevelUpDown)
	{
		const int levelUpCount = m_stats.AddExp(m_stats.GetNextExp());
		if (levelUpCount > 0)
		{
			m_pendingChoices += levelUpCount;
			OpenLevelUpSelect();
		}
		SaveProgress();
	}
	m_prevLevelUpDown = levelUpDown;

	// デバッグ用：F5キーで現在の進行状況を保存する。
	const bool saveDown = (GetAsyncKeyState(VK_F5) & 0x8000);
	if (saveDown && !m_prevSaveDown)
	{
		SaveProgress();
	}
	m_prevSaveDown = saveDown;

	// デバッグ用：F9キーで進行状況を初期状態へ戻す。
	const bool resetDown = (GetAsyncKeyState(VK_F9) & 0x8000);
	if (resetDown && !m_prevResetDown)
	{
		ResetProgress();
	}
	m_prevResetDown = resetDown;

	// デバッグ用：KキーでプレイヤーHPを0にする。
	const bool killDown = (GetAsyncKeyState('K') & 0x8000);
	if (killDown && !m_prevKillDown)
	{
		KillPlayerForDebug();
	}
	m_prevKillDown = killDown;
}

// HPバーの描画処理。
void Status::DrawSprite()
{
	if (m_isChoosingMagic)
	{
		DrawLevelUpSelect();
	}

	if (m_hpBarTex && m_hpBarTex->GetSRView())
	{
		const int barX = HpBarX;
		const int barY = HpBarY;
		const int drawW = HpBarDrawW;
		const int drawH = HpBarDrawH;

		Math::Rectangle srcRect = HpBarSrcRect;
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

		float hpRatio = 0.0f;
		if (m_stats.GetMaxHp() > 0.0f)
		{
			hpRatio = m_stats.GetHp() / m_stats.GetMaxHp();
		}
		hpRatio = std::clamp(hpRatio, 0.0f, 1.0f);

		const int innerX = barX + HpGaugeOffsetX;
		const int innerY = barY + HpGaugeOffsetY;
		const int innerW = HpGaugeW;
		const int innerH = HpGaugeH;

		const int hideW = static_cast<int>(innerW * (1.0f - hpRatio));
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
	}

	DrawExpBar();
	DrawNumber(m_stats.GetLevel(), LevelNumberX, LevelNumberY, NumberDrawW, NumberDrawH);
	DrawVillageGuide();

	if (m_isChoosingMagic)
	{
		DrawCursor();
	}
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

	float expRatio = 0.0f;
	if (m_stats.GetNextExp() > 0.0f)
	{
		expRatio = m_stats.GetExp() / m_stats.GetNextExp();
	}
	expRatio = std::clamp(expRatio, 0.0f, 1.0f);

	const int innerX = ExpBarX + ExpGaugeOffsetX;
	const int innerY = ExpBarY + ExpGaugeOffsetY;
	const int fillW = static_cast<int>(ExpGaugeW * expRatio);
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
		const int digit = text[i] - '0';
		if (digit < 0 || digit > 9) { continue; }

		Math::Rectangle srcRect =
		{
			digit * NumberSrcW,
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

void Status::DrawVillageGuide()
{
	std::shared_ptr<KdGameObject> spPlayer = m_player.lock();
	if (!spPlayer) { return; }

	std::shared_ptr<CameraBase> spCamera = m_camera.lock();
	if (!spCamera) { return; }

	DrawVillageGuideUI(m_villageArrowTex.get(), spPlayer->GetPos(), m_villageGuideRadius, spCamera->WorkCamera().get());
}

void Status::DrawCursor()
{
	DrawCursorUI(m_cursorTex.get());
}
void Status::DrawLevelUpSelect()
{
	LevelUpSelectUITextureSet textures;
	textures.back = m_levelUpBackTex.get();
	textures.icons[0] = m_stats.HasFire() ? m_fireUpTex.get() : m_fireGetTex.get();
	textures.icons[1] = m_stats.HasIce() ? m_iceUpTex.get() : m_iceGetTex.get();
	textures.icons[2] = m_stats.HasVolt() ? m_voltUpTex.get() : m_voltGetTex.get();

	DrawLevelUpSelectUI(textures);
}

void Status::UpdateLevelUpSelect()
{
	const bool leftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
	const int choice = GetClickedLevelUpSelectIndex(leftDown, m_prevLeftDown);

	if (choice == 0)
	{
		PlayUIClickSound();
		EnhanceFire();
		CloseLevelUpSelect();
	}
	else if (choice == 1)
	{
		PlayUIClickSound();
		EnhanceIce();
		CloseLevelUpSelect();
	}
	else if (choice == 2)
	{
		PlayUIClickSound();
		EnhanceVolt();
		CloseLevelUpSelect();
	}
}

void Status::OpenLevelUpSelect()
{
	if (m_pendingChoices <= 0) { return; }

	m_isChoosingMagic = true;
}

void Status::CloseLevelUpSelect()
{
	if (m_pendingChoices > 0)
	{
		--m_pendingChoices;
	}

	m_isChoosingMagic = m_pendingChoices > 0;

	if (!m_isChoosingMagic)
	{
		std::shared_ptr<CameraBase> spCamera = m_camera.lock();
		if (spCamera)
		{
			spCamera->ResetMouseMove();
		}
	}
}

void Status::EnhanceFire()
{
	if (m_stats.HasFire())
	{
		m_stats.EnhanceFire();
	}
	else
	{
		m_stats.UnlockFire();
	}
	SaveProgress();
}

void Status::EnhanceIce()
{
	if (m_stats.HasIce())
	{
		m_stats.EnhanceIce();
	}
	else
	{
		m_stats.UnlockIce();
	}
	SaveProgress();
}

void Status::EnhanceVolt()
{
	if (m_stats.HasVolt())
	{
		m_stats.EnhanceVolt();
	}
	else
	{
		m_stats.UnlockVolt();
	}
	SaveProgress();
}

// プレイヤーHPを減らす処理。
void Status::DamagePlayer(float damage)
{
	m_stats.Damage(damage);
}

// プレイヤーHPを最大まで戻す処理。
void Status::ResetPlayerHp()
{
	m_stats.ResetHp();
}

// 経験値を加算する処理。
void Status::AddExp(float exp)
{
	const int levelUpCount = m_stats.AddExp(exp);
	if (levelUpCount <= 0)
	{
		SaveProgress();
		return;
	}

	m_pendingChoices += levelUpCount;
	OpenLevelUpSelect();
	SaveProgress();
}

void Status::SaveProgress()
{
	std::filesystem::create_directories("Save");

	const PlayerStatus::SaveData data = m_stats.GetSaveData();
	std::ofstream file(ProgressSavePath);
	if (!file) { return; }

	file << data.hp << '\n';
	file << data.maxHp << '\n';
	file << data.mp << '\n';
	file << data.attack << '\n';
	file << data.defense << '\n';
	file << data.speed << '\n';
	file << data.level << '\n';
	file << data.exp << '\n';
	file << data.nextExp << '\n';
	file << data.fireExplosionRadius << '\n';
	file << data.iceSplitCount << '\n';
	file << data.icePierceCount << '\n';
	file << data.voltChainCount << '\n';
	file << data.hasFire << '\n';
	file << data.hasIce << '\n';
	file << data.hasVolt << '\n';
}

void Status::LoadProgress()
{
	std::ifstream file(ProgressSavePath);
	if (!file) { return; }

	PlayerStatus::SaveData data;
	file >> data.hp;
	file >> data.maxHp;
	file >> data.mp;
	file >> data.attack;
	file >> data.defense;
	file >> data.speed;
	file >> data.level;
	file >> data.exp;
	file >> data.nextExp;
	file >> data.fireExplosionRadius;
	file >> data.iceSplitCount;
	file >> data.icePierceCount;
	file >> data.voltChainCount;
	if (!file) { return; }

	// 古い保存データには魔法取得フラグが無い可能性があるため、読める時だけ反映する。
	bool hasFire = data.hasFire;
	bool hasIce = data.hasIce;
	bool hasVolt = data.hasVolt;
	if (file >> hasFire >> hasIce >> hasVolt)
	{
		data.hasFire = hasFire;
		data.hasIce = hasIce;
		data.hasVolt = hasVolt;
	}

	m_stats.ApplySaveData(data);
}

void Status::ResetProgress()
{
	m_stats.Reset();
	m_pendingChoices = 1;
	OpenLevelUpSelect();

	std::error_code error;
	std::filesystem::remove(ProgressSavePath, error);
}

void Status::KillPlayerForDebug()
{
	m_stats.Damage(m_stats.GetMaxHp());
}

bool Status::HasMagic(MagicType type) const
{
	switch (type)
	{
	case MagicType::Fire:
		return m_stats.HasFire();
	case MagicType::Ice:
		return m_stats.HasIce();
	case MagicType::Volt:
		return m_stats.HasVolt();
	default:
		return false;
	}
}
