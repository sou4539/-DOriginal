#pragma once

class PlayerStatus
{
public:
	struct SaveData
	{
		float hp = 100.0f;
		float maxHp = 100.0f;
		float mp = 100.0f;
		float attack = 10.0f;
		float defense = 5.0f;
		float speed = 1.0f;
		int level = 1;
		float exp = 0.0f;
		float nextExp = 100.0f;
		float fireExplosionRadius = 3.0f;
		int iceSplitCount = 1;
		int icePierceCount = 1;
		int voltChainCount = 1;
		bool hasFire = false;
		bool hasIce = false;
		bool hasVolt = false;
	};

	void Damage(float damage);
	void ResetHp();
	void Reset();

	bool IsDead() const { return m_hp <= 0.0f; }

	float GetHp() const { return m_hp; }
	float GetMaxHp() const { return m_maxHp; }
	int GetLevel() const { return m_level; }
	float GetExp() const { return m_exp; }
	float GetNextExp() const { return m_nextExp; }

	float GetFireExplosionRadius() const { return m_fireExplosionRadius; }
	int GetIceSplitCount() const { return m_iceSplitCount; }
	int GetIcePierceCount() const { return m_icePierceCount; }
	int GetVoltChainCount() const { return m_voltChainCount; }
	bool HasFire() const { return m_hasFire; }
	bool HasIce() const { return m_hasIce; }
	bool HasVolt() const { return m_hasVolt; }
	bool HasAnyMagic() const { return m_hasFire || m_hasIce || m_hasVolt; }

	int AddExp(float exp);
	void UnlockFire();
	void UnlockIce();
	void UnlockVolt();
	void EnhanceFire();
	void EnhanceIce();
	void EnhanceVolt();
	SaveData GetSaveData() const;
	void ApplySaveData(const SaveData& data);

private:
	void LevelUp();

	float m_hp = 100.0f;
	float m_maxHp = 100.0f;
	float m_mp = 100.0f;
	float m_attack = 10.0f;
	float m_defense = 5.0f;
	float m_speed = 1.0f;

	int m_level = 1;
	float m_exp = 0.0f;
	float m_nextExp = 100.0f;

	float m_fireExplosionRadius = 3.0f;
	int m_iceSplitCount = 1;
	int m_icePierceCount = 1;
	int m_voltChainCount = 1;
	bool m_hasFire = false;
	bool m_hasIce = false;
	bool m_hasVolt = false;
};
