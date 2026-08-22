#pragma once

#include "../StageBase.h"

class Village :public StageBase
{
public:
	Village() { Init(); }
	~Village() override {}

	void Update() override;
	void DrawLit() override;

	// 村は壁判定にも使う。
	bool EnableSphereCollision() const override { return true; }

	// 安全地帯の中心座標。
	const Math::Vector3& GetSafeAreaCenter() const { return m_safeAreaCenter; }

	// 安全地帯の半径。
	float GetSafeAreaRadius() const { return m_safeAreaRadius; }

	// 村の表示判定に使う対象を設定する。
	void SetTarget(const std::shared_ptr<KdGameObject>& target)
	{
		m_wpTarget = target;
	}

	// 村を表示する距離を設定する。
	void SetVisibleRadius(float radius) { m_visibleRadius = radius; }

private:
	void Init() override;

	// 村が表示範囲内か確認する。
	bool IsInVisibleRange() const;

	// 村全体を覆う安全地帯。
	Math::Vector3 m_safeAreaCenter = Math::Vector3::Zero;
	float m_safeAreaRadius = 46.0f;

	// 表示距離を確認する対象。
	std::weak_ptr<KdGameObject> m_wpTarget;

	// 村を表示する半径。
	float m_visibleRadius = 85.0f;
};


