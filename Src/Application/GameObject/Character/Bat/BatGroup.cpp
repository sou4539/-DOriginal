#include "BatGroup.h"

#include "Bat.h"
#include "../Player/Player.h"
#include <algorithm>

// デバッグ表示用のワイヤを準備する。
void BatGroup::Init()
{
	m_pDebugWire = std::make_unique<KdDebugWireFrame>();
}

// 出現範囲の表示と、コウモリ数の維持を行う。
void BatGroup::Update()
{
	// 白が最大出現距離、赤が出現させない近距離。
	if (m_pDebugWire)
	{
		std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
		if (spTarget)
		{
			Math::Vector3 debugCenter = spTarget->GetPos();
			debugCenter.y = 3.0f;
			m_pDebugWire->AddDebugSphere(debugCenter, m_spawnMaxRadius, kWhiteColor);
			m_pDebugWire->AddDebugSphere(debugCenter, m_spawnMinRadius, kRedColor);
		}
	}

	UpdateBatCountByDistance();
	MaintainBatCount();
}

// 生成するコウモリ全体の追跡対象を設定する。
void BatGroup::SetTarget(const std::shared_ptr<KdGameObject>& target)
{
	m_wpTarget = target;
}

// コウモリの生息地情報を追加する。
void BatGroup::AddHabitat(const Math::Vector3& center, float radius, int count)
{
	Habitat habitat;
	habitat.center = center;
	habitat.radius = radius;
	habitat.count = count;

	m_habitats.push_back(habitat);
}

// 登録済みの生息地情報をもとに、初期コウモリをシーンへ追加する。
void BatGroup::AddBatsToScene(std::list<std::shared_ptr<KdGameObject>>& objList, const std::shared_ptr<KdGameObject>& target)
{
	SetTarget(target);
	m_pObjList = &objList;
	m_bats.clear();
	UpdateBatCountByDistance();

	for (int habitatIndex = 0; habitatIndex < static_cast<int>(m_habitats.size()); ++habitatIndex)
	{
		const Habitat& habitat = m_habitats[habitatIndex];

		for (int i = 0; i < habitat.count; ++i)
		{
			if (GetTotalBatCount() >= m_nowMaxBatCount) { return; }

			AddBatToScene(objList, target, habitatIndex);
		}
	}
}

void BatGroup::UpdateBatCountByDistance()
{
	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
	if (!spTarget) { return; }

	Math::Vector3 toPlayer = spTarget->GetPos() - m_safeAreaCenter;
	toPlayer.y = 0.0f;

	float distance = toPlayer.Length() - m_safeAreaRadius;
	distance = std::max(distance, 0.0f);

	float rate = distance / m_maxBatCountDistance;
	rate = std::clamp(rate, 0.0f, 1.0f);

	m_nowMaxBatCount = static_cast<int>(m_minBatCount + (m_maxBatCount - m_minBatCount) * rate);
}

// プレイヤー周辺かつ村の外になるランダム出現座標を作る。
Math::Vector3 BatGroup::MakeRandomPos(int habitatIndex) const
{
	if (habitatIndex < 0 || habitatIndex >= static_cast<int>(m_habitats.size()))
	{
		return Math::Vector3::Zero;
	}

	const Habitat& habitat = m_habitats[habitatIndex];
	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
	if (!spTarget)
	{
		return habitat.center;
	}

	const Math::Vector3 targetPos = spTarget->GetPos();
	Math::Vector3 pos = targetPos;
	Math::Vector3 habitatDir = habitat.center - m_safeAreaCenter;
	habitatDir.y = 0.0f;

	float baseAngle = KdRandom::GetFloat(0.0f, DirectX::XM_2PI);
	if (habitatDir.LengthSquared() > 0.0001f)
	{
		baseAngle = std::atan2(habitatDir.z, habitatDir.x);
	}

	for (int i = 0; i < 30; ++i)
	{
		// 生息地ごとの方向を基準に、少しだけ散らしてまとまった群れにする。
		float angle = baseAngle + KdRandom::GetFloat
		(
			-DirectX::XMConvertToRadians(18.0f),
			DirectX::XMConvertToRadians(18.0f)
		);

		// プレイヤーから少し離れた位置に、まとまりを保ったまま出す。
		float radiusRate = std::sqrt(KdRandom::GetFloat(0.0f, 1.0f));
		float radius = m_spawnMinRadius + (m_spawnMaxRadius - m_spawnMinRadius) * radiusRate;

		pos.x = targetPos.x + std::cos(angle) * radius;
		pos.y = habitat.center.y;
		pos.z = targetPos.z + std::sin(angle) * radius;

		if (!IsInSafeArea(pos))
		{
			return pos;
		}
	}

	// 抽選に失敗した時は、プレイヤーから離れた位置へ出す。
	pos.x = targetPos.x - m_spawnMaxRadius;
	pos.y = habitat.center.y;
	pos.z = targetPos.z;

	return pos;
}

bool BatGroup::IsInSafeArea(const Math::Vector3& pos) const
{
	if (m_safeAreaRadius <= 0.0f) { return false; }

	Math::Vector3 toPos = pos - m_safeAreaCenter;
	toPos.y = 0.0f;

	return toPos.LengthSquared() <= m_safeAreaRadius * m_safeAreaRadius;
}

bool BatGroup::IsOutsideSpawnSphere(const Math::Vector3& pos) const
{
	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
	if (!spTarget) { return false; }

	Math::Vector3 toPos = pos - spTarget->GetPos();
	toPos.y = 0.0f;

	return toPos.LengthSquared() > m_spawnMaxRadius * m_spawnMaxRadius;
}

// 消えたコウモリを整理し、足りない分を1フレーム1体ずつ補充する。
void BatGroup::MaintainBatCount()
{
	std::vector<BatInfo> liveBats;

	for (BatInfo& batInfo : m_bats)
	{
		std::shared_ptr<Bat> spBat = batInfo.bat.lock();
		if (!spBat) { continue; }
		if (spBat->IsExpired()) { continue; }

		// 出現スフィア外まで離れたコウモリは消す。
		if (IsOutsideSpawnSphere(spBat->GetPos()))
		{
			spBat->Expire();
			continue;
		}

		// 生きているコウモリだけ管理リストへ残す。
		liveBats.push_back(batInfo);
	}

	m_bats = liveBats;

	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
	if (!m_pObjList || !spTarget) { return; }

	if (GetTotalBatCount() >= m_nowMaxBatCount) { return; }

	// 生息地ごとの数が設定数に戻るように補充する。
	for (int habitatIndex = 0; habitatIndex < static_cast<int>(m_habitats.size()); ++habitatIndex)
	{
		int batCountInHabitat = 0;
		for (const BatInfo& batInfo : m_bats)
		{
			if (batInfo.habitatIndex == habitatIndex)
			{
				++batCountInHabitat;
			}
		}

		if (batCountInHabitat < m_habitats[habitatIndex].count)
		{
			AddBatToScene(*m_pObjList, spTarget, habitatIndex);

			if (m_isSpawnOnePerFrame)
			{
				return;
			}
		}
	}
}

// コウモリを1体作成し、初期位置や追跡対象を設定してシーンへ追加する。
void BatGroup::AddBatToScene(std::list<std::shared_ptr<KdGameObject>>& objList, const std::shared_ptr<KdGameObject>& target, int habitatIndex)
{
	Math::Vector3 startPos = MakeRandomPos(habitatIndex);

	std::shared_ptr<Bat> bat = std::make_shared<Bat>();
	bat->SetStartPos(startPos);
	bat->SetAngle(KdRandom::GetFloat(0.0f, DirectX::XM_2PI));
	bat->SetTarget(target);

	std::shared_ptr<Status> spStatus = m_wpStatus.lock();
	if (spStatus)
	{
		bat->SetStatus(spStatus);
	}

	objList.push_back(bat);

	BatInfo batInfo;
	batInfo.bat = bat;
	batInfo.habitatIndex = habitatIndex;
	m_bats.push_back(batInfo);
}



















