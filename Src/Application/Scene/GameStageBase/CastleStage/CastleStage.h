#pragma once

#include "../GameStageBase.h"
#include "../StageTransition.h"

#include <functional>

class CastleExit;
class GrudgeBat;
struct GrudgeBattleState;

class CastleStage : public GameStageBase
{
public:

	// Statusを受け取った後、Startから初期化する。
	CastleStage() = default;
	~CastleStage() override{}

	void SetExitCallback(const std::function<void()>& callback) { m_onExit = callback; }
	void SetDeathCallback(const std::function<void()>& callback) { m_onDeath = callback; }
	void SetEntryRoute(CastleEntryRoute route) { m_entryRoute = route; }
	void SetGrudgeBattle(const std::shared_ptr<GrudgeBattleState>& battle) { m_grudgeBattle = battle; }
	void SetPortalReturnCallback(const std::function<void()>& callback) { m_onPortalReturn = callback; }

private:
	void Init() override;
	void Event() override;
	InteractionAction GetInteractionAction() const override;
	std::function<void()> m_onExit;
	std::function<void()> m_onDeath;
	CastleEntryRoute m_entryRoute = CastleEntryRoute::Entrance;
	std::function<void()> m_onPortalReturn;
	std::shared_ptr<CastleExit> m_exit;
	bool m_prevExitDown = false;
	bool m_prevDebugSpawnDown = false;

	// 指定数を追加する。倒した怨念は自動補充しない。
	void CreateGrudges(int count, bool addToTotal = true);
	void UpdateActiveGrudges();
	void RefillGrudges();
	bool CanRecall() const override { return false; }

	std::shared_ptr<GrudgeBattleState> m_grudgeBattle;
	std::vector<std::weak_ptr<GrudgeBat>> m_grudges;
};
