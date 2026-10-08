#pragma once

#include "../StageBase.h"

// 城内のモデル。壁との球判定でプレイヤーを押し戻す。
class Castle : public StageBase
{
public:
	Castle() { Init(); }
	~Castle() override = default;
	void DrawLit() override;
	void GenerateDepthMapFromLight() override;

	// StageBaseは標準で球判定を省略するので、城の壁では有効にする。
	bool EnableSphereCollision() const override { return true; }

	// 壁を抜く距離をプレイヤーとカメラの距離に合わせるために使う。
	void SetTarget(const std::shared_ptr<KdGameObject>& target) { m_wpTarget = target; }

private:
	void Init() override;
	std::weak_ptr<KdGameObject> m_wpTarget;
};
