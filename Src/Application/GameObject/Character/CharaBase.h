#pragma once

class CharaBase :public KdGameObject
{
public:
	CharaBase() {}
	~CharaBase() override { Release(); }
	void Init() override;
	virtual void Update() override;
	void PostUpdate() override;
	void DrawLit() override;

	void RegistHitObject(const std::shared_ptr<KdGameObject>& object)
	{
		m_wpHitObjectList.push_back(object);
	}

private:
	// 衝突判定とそれに伴う座標の更新
	void UpdateCollision();

	// 解放処理
	void Release();

protected:
	std::shared_ptr<KdSquarePolygon>			m_spPoly = nullptr;
	std::shared_ptr<KdModelWork>				m_spModel = nullptr;
	std::vector<std::weak_ptr<KdGameObject>>	m_wpHitObjectList{};
	float										m_Gravity = 0;

	//乗り物制御関係
	Math::Matrix                                m_mLocalFromRideObject;
	std::weak_ptr<KdGameObject>                 m_wpRiddenObject;

	Math::Vector3 m_pos;
	Math::Vector3 m_dir;
};