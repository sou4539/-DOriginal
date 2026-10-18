#pragma once

#include "../GameStageBase.h"
#include "../StageTransition.h"

#include <functional>

class NoneCastle;

class GrassStage : public GameStageBase
{
public:

	// Statusを受け取った後、Startから初期化する。
	GrassStage() = default;
	~GrassStage() override{}

	// 入場条件を満たした時に呼ぶ処理を、GameSceneから受け取る。
	void SetCastleEntryCallback(const std::function<void(CastleEntryRoute)>& callback)
	{
		m_onCastleEntry = callback;
	}

	void SetArrivalPoint(GrassArrivalPoint arrival) { m_arrivalPoint = arrival; }
	void PostUpdate() override;
private:
	void Init() override;
	// Bのデバッグワープと、入口でのFキーの入場判定を行う。
	void Event() override;
	InteractionAction GetInteractionAction() const override;
	bool IsNearVillagePortal() const;
	std::weak_ptr<NoneCastle> m_castle;
	GrassArrivalPoint m_arrivalPoint = GrassArrivalPoint::VillageSpawn;
	bool m_prevWarpDown = false;
	bool m_prevEnterDown = false;
	std::function<void(CastleEntryRoute)> m_onCastleEntry;

};
