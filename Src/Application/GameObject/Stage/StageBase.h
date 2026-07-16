#pragma once

class StageBase : public KdGameObject
{
public:
	StageBase(){}
	~StageBase() override {}

	void DrawLit() override;

	// 壁・障害物用のスフィア判定対象にするかどうか。
	// Groundのような広い地面はfalseのままにして、重い球 vs モデル判定を避ける。
	virtual bool EnableSphereCollision() const { return false; }

protected:

	std::shared_ptr<KdModelWork> m_spModel = nullptr;
};
