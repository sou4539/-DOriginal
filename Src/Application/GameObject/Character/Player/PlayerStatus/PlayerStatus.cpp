#include "PlayerStatus.h"

#include <algorithm>

void PlayerStatus::Damage(float damage)
{
	if (damage <= 0.0f) { return; }

	m_hp -= damage;
	if (m_hp < 0.0f)
	{
		m_hp = 0.0f;
	}
}

void PlayerStatus::ResetHp()
{
	m_hp = m_maxHp;
}

void PlayerStatus::Reset()
{
	m_hp = 100.0f;
	m_maxHp = 100.0f;
	m_mp = 100.0f;
	m_attack = 10.0f;
	m_defense = 5.0f;
	m_speed = 1.0f;
	m_level = 1;
	m_exp = 0.0f;
	m_nextExp = 100.0f;
	m_fireExplosionRadius = 3.0f;
	m_iceSplitCount = 2;
	m_icePierceCount = 1;
	m_voltChainCount = 1;
	m_hasFire = false;
	m_hasIce = false;
	m_hasVolt = false;
}

int PlayerStatus::AddExp(float exp)
{
	if (exp <= 0.0f) { return 0; }

	m_exp += exp;

	int levelUpCount = 0;
	while (m_exp >= m_nextExp)
	{
		m_exp -= m_nextExp;
		LevelUp();
		++levelUpCount;
	}

	return levelUpCount;
}

void PlayerStatus::LevelUp()
{
	++m_level;

	m_maxHp += 10.0f;
	m_attack += 2.0f;
	m_hp = m_maxHp;

	m_nextExp *= 1.25f;
}

void PlayerStatus::UnlockFire()
{
	m_hasFire = true;
}

void PlayerStatus::UnlockIce()
{
	m_hasIce = true;
}

void PlayerStatus::UnlockVolt()
{
	m_hasVolt = true;
}

void PlayerStatus::EnhanceFire()
{
	m_fireExplosionRadius += 0.5f;
}

void PlayerStatus::EnhanceIce()
{
	++m_iceSplitCount;
	++m_icePierceCount;
}

void PlayerStatus::EnhanceVolt()
{
	++m_voltChainCount;
}

PlayerStatus::SaveData PlayerStatus::GetSaveData() const
{
	SaveData data;
	data.hp = m_hp;
	data.maxHp = m_maxHp;
	data.mp = m_mp;
	data.attack = m_attack;
	data.defense = m_defense;
	data.speed = m_speed;
	data.level = m_level;
	data.exp = m_exp;
	data.nextExp = m_nextExp;
	data.fireExplosionRadius = m_fireExplosionRadius;
	data.iceSplitCount = m_iceSplitCount;
	data.icePierceCount = m_icePierceCount;
	data.voltChainCount = m_voltChainCount;
	data.hasFire = m_hasFire;
	data.hasIce = m_hasIce;
	data.hasVolt = m_hasVolt;
	return data;
}

void PlayerStatus::ApplySaveData(const SaveData& data)
{
	m_maxHp = std::max(data.maxHp, 1.0f);
	m_hp = std::clamp(data.hp, 0.0f, m_maxHp);
	m_mp = std::max(data.mp, 0.0f);
	m_attack = std::max(data.attack, 0.0f);
	m_defense = std::max(data.defense, 0.0f);
	m_speed = std::max(data.speed, 0.0f);
	m_level = std::max(data.level, 1);
	m_exp = std::max(data.exp, 0.0f);
	m_nextExp = std::max(data.nextExp, 1.0f);
	m_fireExplosionRadius = std::max(data.fireExplosionRadius, 0.0f);
	m_iceSplitCount = std::max(data.iceSplitCount, 2);
	m_icePierceCount = std::max(data.icePierceCount, 1);
	m_voltChainCount = std::max(data.voltChainCount, 0);
	m_hasFire = data.hasFire;
	m_hasIce = data.hasIce;
	m_hasVolt = data.hasVolt;
}
