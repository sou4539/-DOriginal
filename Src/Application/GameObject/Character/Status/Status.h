#pragma once

class Status : public KdGameObject
{
public:
	Status() {}
	~Status() {}
	void Init();
	void Update();

	void SetPlayer(const std::weak_ptr<KdGameObject>& player) { m_player = player; }
	void SetEnemy(const std::weak_ptr<KdGameObject>& enemy) { m_enemy = enemy; }
private:
	std::weak_ptr<KdGameObject> m_player;
	std::weak_ptr<KdGameObject> m_enemy;

	//各種ステータス
	//プレイヤー
	float m_pHp = 100.0f;
	float m_pMp = 100.0f;
	float m_pAttack = 10.0f;
	float m_pDefense = 5.0f;
	float m_pSpeed = 1.0f;

	//敵
	float m_eHp = 50.0f;
	float m_eMp = 50.0f;
	float m_eAttack = 5.0f;
	float m_eDefense = 2.0f;
	float m_eSpeed = 0.5f;
};