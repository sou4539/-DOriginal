#pragma once

#include "../GameStageBase.h"

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

private:
	void Init() override;
	void Event() override;
	std::function<void()> m_onExit;
	std::shared_ptr<CastleExit> m_exit;
	bool m_prevExitDown = false;
	// 初期出現のみ。倒した怨念は補充せず、全滅できるようにする。
	void CreateGrudges(int count);
	void UpdateActiveGrudges();
	std::shared_ptr<GrudgeBattleState> m_grudgeBattle;
	std::vector<std::weak_ptr<GrudgeBat>> m_grudges;
	std::shared_ptr<KdSquarePolygon> m_gate;
};
