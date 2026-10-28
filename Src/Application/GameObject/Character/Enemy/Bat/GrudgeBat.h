#pragma once
#include "Bat.h"
#include "../../Status/GameBalance.h"

// GameSceneが保持し、城を離れても残数と強化を引き継ぐ。
struct GrudgeBattleState
{
	int TotalCount = 0;
	int DefeatedCount = 0;
	// 撃破累計とは分ける。退城で倍率を下げても残数の計算は変えない。
	float StrengthBonus = 0.0f;
	int GetRemainingCount() const { return std::max(0, TotalCount - DefeatedCount); }
	float GetMultiplier() const { return 1.0f + StrengthBonus; }
	void OnCastleExit()
	{
		StrengthBonus = std::max(0.0f, StrengthBonus - GameBalance::GrudgeStrengthLossPerExit);
	}
};

// コウモリの怨念。通常のコウモリの挙動を再利用し、能力値と報酬を変える。
class GrudgeBat : public Bat
{
public:
	explicit GrudgeBat(const std::shared_ptr<GrudgeBattleState>& battle);
	void Update() override;
	void OnHit() override { OnHit(10.0f); }
	void OnHit(float _damage,MagicType type=MagicType::None) override;
	void DrawLit() override;
	// 大量出現時の負荷を抑えるため、怨念は影用の描画へ登録しない。
	void GenerateDepthMapFromLight() override {}
	float GetContactDamage() const override;

protected:
	// 怨念を倒しても、次回の城用の草原撃破数には含めない。
	bool CountsAsGrassBat() const override { return false; }
	// 出現直後から、距離や安全地帯に関係なくプレイヤーを追尾する。
	bool AlwaysChases() const override { return true; }
	void OnDefeated() override;

private:
	void RefreshStrength();
	std::shared_ptr<GrudgeBattleState> m_battle;
	int m_appliedDefeatedCount = -1;
};
