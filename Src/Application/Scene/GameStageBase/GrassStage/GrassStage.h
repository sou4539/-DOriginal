#pragma once

#include "../GameStageBase.h"

#include <functional>

class NoneCastle;

class GrassStage : public GameStageBase
{
public:

	// Statusを受け取った後、Startから初期化する。
	GrassStage() = default;
	~GrassStage() override{}

	// 入場条件を満たした時に呼ぶ処理を、GameSceneから受け取る。
	void SetCastleEntryCallback(const std::function<void()>& callback)
	{
		m_onCastleEntry = callback;
	}

	void SetReturnFromCastle(bool value) { m_returnFromCastle = value; }

private:
	void Init() override;
	// Bのデバッグワープと、入口でのEキーの入場判定を行う。
	void Event() override;
	std::weak_ptr<NoneCastle> m_castle;
	bool m_returnFromCastle = false;
	bool m_prevWarpDown = false;
	bool m_prevEnterDown = false;
	std::function<void()> m_onCastleEntry;

};
