#pragma once

#include "../StageBase.h"

class Village :public StageBase
{
public:
	Village() { Init(); }
	~Village() override {}

	void Update() override;

	// 村のCOLは壁・柵としても使うため、スフィア判定対象にする。
	bool EnableSphereCollision() const override { return true; }

	// 村を覆う安全地帯スフィアの中心座標を返す。
	const Math::Vector3& GetSafeAreaCenter() const { return m_safeAreaCenter; }

	// 村を覆う安全地帯スフィアの半径を返す。
	float GetSafeAreaRadius() const { return m_safeAreaRadius; }

private:
	void Init() override;

	// village.gltfのmin/maxから、村全体を覆うように設定した安全地帯。
	Math::Vector3 m_safeAreaCenter = { -0.12f, 0.0f, -14.23f };
	float m_safeAreaRadius = 46.0f;
};
