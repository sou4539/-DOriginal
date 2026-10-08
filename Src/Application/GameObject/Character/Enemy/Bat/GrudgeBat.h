#pragma once
#include "Bat.h"

// 城への入場ごとに作り直す共有状態。草原の撃破数やStatusとは独立している。
struct GrudgeBattleState
{
	int TotalCount = 0;
	int DefeatedCount = 0;
	// 毎回の乗算ではなく、初期値へ5%ずつ加算する。
	float GetMultiplier() const { return 1.0f + 0.05f * DefeatedCount; }
};

// コウモリの怨念。通常のコウモリの挙動を再利用し、能力値と報酬を変える。
class GrudgeBat : public Bat
{
public:
	explicit GrudgeBat(const std::shared_ptr<GrudgeBattleState>& battle);
	void Update() override;
	void OnHit() override { OnHit(10.0f); }
	void OnHit(float damage) override;
	void DrawLit() override;
	float GetContactDamage() const override;

protected:
	// 怨念を倒しても、次回の城用の草原撃破数には含めない。
	bool CountsAsGrassBat() const override { return false; }
	bool AlwaysChases() const override { return true; }
	void OnDefeated() override;

private:
	void RefreshStrength();
	std::shared_ptr<GrudgeBattleState> m_battle;
	int m_appliedDefeatedCount = -1;
};
