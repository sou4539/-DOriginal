#pragma once

#include "../../UI/UI.h"
#include "../Player/PlayerStatus/PlayerStatus.h"

class CameraBase;
enum class MagicType;

class Status : public UI
{
public:
	// 生成時にUI画像を読み込む。
	Status() { Init(); }
	~Status() override {}

	// UI画像を読み込む。
	void Init();

	// ステータスとレベルアップ選択を更新する。
	void Update();

	// HP、経験値、レベル、村案内を描画する。
	void DrawSprite() override;

	// UIで参照する対象を設定する。
	void SetPlayer(const std::weak_ptr<KdGameObject>& player) { m_player = player; }
	void SetCamera(const std::weak_ptr<CameraBase>& camera) { m_camera = camera; }
	void SetEnemy(const std::weak_ptr<KdGameObject>& enemy) { m_enemy = enemy; }

	// 村案内を消す半径を設定する。
	void SetVillageGuideRadius(float radius) { m_villageGuideRadius = radius; }

	// プレイヤーHPを減らす。
	void DamagePlayer(float damage);

	// プレイヤーHPを最大まで戻す。
	void ResetPlayerHp();

	// HPが0か確認する。
	bool IsPlayerDead() const { return m_stats.IsDead(); }
	float GetPlayerHp() const { return m_stats.GetHp(); }

	// 経験値を加算する。
	void AddExp(float exp);

	int GetLevel() const { return m_stats.GetLevel(); }
	float GetExp() const { return m_stats.GetExp(); }
	float GetNextExp() const { return m_stats.GetNextExp(); }
	float GetPlayerAttack() const { return m_stats.GetAttack(); }

	// 魔法強化値を返す。
	float GetFireExplosionRadius() const { return m_stats.GetFireExplosionRadius(); }
	int GetIceSplitCount() const { return m_stats.GetIceSplitCount(); }
	int GetIcePierceCount() const { return m_stats.GetIcePierceCount(); }
	int GetVoltChainCount() const { return m_stats.GetVoltChainCount(); }
	bool HasFire() const { return m_stats.HasFire(); }
	bool HasIce() const { return m_stats.HasIce(); }
	bool HasVolt() const { return m_stats.HasVolt(); }
	bool HasAnyMagic() const { return m_stats.HasAnyMagic(); }
	bool HasMagic(MagicType type) const;
	bool IsLevelUpSelect() const { return m_isChoosingMagic; }

	// 倒したコウモリの数を加算する。
	void AddKillBatCount()
	{ 
		++m_KillBatCount;
	}

	// 倒したコウモリの数を取得する。
	int GetKillBatCount() const
	{
		return m_KillBatCount;
	}
	
	// 倒したコウモリの数をリセットする。
	void ResetKillBatCount()
	{
		m_KillBatCount = 0;
	}

private:
	void SaveProgress();
	void LoadProgress();
	void ResetProgress();
	void KillPlayerForDebug();
	void UpdateDebugKeys();

	void OpenLevelUpSelect();
	void CloseLevelUpSelect();

	// レベルアップ選択UIを描画する。
	void DrawLevelUpSelect();

	// レベルアップ選択UIの入力を処理する。
	void UpdateLevelUpSelect();
	void EnhanceFire();
	void EnhanceIce();
	void EnhanceVolt();
	void DrawExpBar();
	void DrawNumber(int value, int x, int y, int drawW, int drawH);
	void DrawVillageGuide();
	void DrawCursor();

	// UI描画や判定で参照する対象。
	std::weak_ptr<KdGameObject> m_player;
	std::weak_ptr<KdGameObject> m_enemy;
	std::weak_ptr<CameraBase> m_camera;

	// UI画像。
	std::shared_ptr<KdTexture> m_hpBarTex = nullptr;
	std::shared_ptr<KdTexture> m_levelUpBackTex = nullptr;
	std::shared_ptr<KdTexture> m_fireUpTex = nullptr;
	std::shared_ptr<KdTexture> m_iceUpTex = nullptr;
	std::shared_ptr<KdTexture> m_voltUpTex = nullptr;
	std::shared_ptr<KdTexture> m_fireGetTex = nullptr;
	std::shared_ptr<KdTexture> m_iceGetTex = nullptr;
	std::shared_ptr<KdTexture> m_voltGetTex = nullptr;
	std::shared_ptr<KdTexture> m_numberTex = nullptr;
	std::shared_ptr<KdTexture> m_villageArrowTex = nullptr;
	std::shared_ptr<KdTexture> m_cursorTex = nullptr;

	// 原点からこの距離内では村案内を表示しない。
	float m_villageGuideRadius = 0.0f;

	// プレイヤーの数値管理。
	PlayerStatus m_stats;

	// レベルアップ選択UIの状態。
	bool m_isChoosingMagic = false;
	int m_pendingChoices = 0;

	// 入力の押しっぱなし防止。
	bool m_prevLevelUpDown = false;
	bool m_prevSaveDown = false;
	bool m_prevResetDown = false;
	bool m_prevKillDown = false;
	bool m_prevLeftDown = false;

	// 倒したコウモリの数
	// これがそのまま敵の数にもなる
	int m_KillBatCount = 0;
};
