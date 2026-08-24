#include "MagicBase.h"

#include "../../../Scene/SceneManager.h"
#include "../Enemy/EnemyBase.h"

#include <algorithm>

namespace
{
	// 2.5DOriginalのdと同じ考え方。
	constexpr float MagicChantSpeed = 0.05f;
}

void MagicBase::Init()
{
	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

	m_pos = {};
	m_dir = {};
	m_magicType = MagicType::None;
	m_state = MagicState::Chant;
	m_damage = 0.0f;
	m_speed = 0.0f;
	m_lifeTime = 0.0f;
	m_radius = 0.0f;
	m_chant = 1.0f;
	m_chantSpeed = MagicChantSpeed;
	m_framePathList.clear();
	m_frame = 0.0f;
	m_frameSpeed = 0.0f;
	m_nowFrame = -1;
	m_wpChantTarget.reset();
	m_chantOffset = Math::Vector3::Zero;
	m_wpFlyTarget.reset();
	m_wpIgnoreTarget.reset();
	m_hitObjectList.clear();

	m_spPoly = std::make_shared<KdSquarePolygon>();
}

void MagicBase::Update()
{
	switch (m_state)
	{
	case MagicState::Chant:
		UpdateChant();
		break;
	case MagicState::Fly:
		UpdateFly();
		break;
	case MagicState::Hit:
		UpdateHit();
		break;
	default:
		break;
	}

	if (m_pDebugWire)
	{
		// 当たり判定の確認が必要な時だけコメントアウトを外す。
	}

	UpdateWorldMatrix();
}

void MagicBase::UpdateChant()
{
	// 詠唱中だけ、Shot()で受け取った対象を追いかける。
	if (auto spChantTarget = m_wpChantTarget.lock())
	{
		m_pos = spChantTarget->GetPos() + m_chantOffset;
	}

	// 詠唱中は2.5DOriginalと同じく、値を1.0から0.0へ減らしていく。
	m_chant -= m_chantSpeed;

	UpdateChantMagic();

	if (IsReadyToFly())
	{
		StartFly();
	}
}

void MagicBase::UpdateFly()
{
	// 飛行中だけ寿命を減らし、進行方向へ移動する。
	m_lifeTime -= 1.0f;
	if (m_lifeTime <= 0.0f)
	{
		m_isExpired = true;
		return;
	}

	m_pos += m_dir * m_speed;

	UpdateFlyMagic();
}

void MagicBase::UpdateHit()
{
	UpdateHitMagic();
}

void MagicBase::StartFly()
{
	// 詠唱完了後、実際に魔法弾が飛び始める瞬間の処理。
	m_wpChantTarget.reset();

	// 詠唱中に敵や杖が動いた場合に備えて、
	if (auto spFlyTarget = m_wpFlyTarget.lock())
	{
		Math::Vector3 flyDir = spFlyTarget->GetPos() - m_pos;
		if (flyDir.LengthSquared() > 0.0001f)
		{
			flyDir.Normalize();
			m_dir = flyDir;
		}
	}

	m_chant = 0.0f;
	m_state = MagicState::Fly;

	PlayShotSound();
}

void MagicBase::StartHit()
{
	// 敵に当たった瞬間の共通処理。
	PlayHitSound();

	if (StartHitAnimation())
	{
		m_state = MagicState::Hit;
	}
	else
	{
		m_isExpired = true;
	}
}

void MagicBase::PostUpdate()
{
	// 詠唱中と命中演出中は、まだ敵へ当てない。
	if (m_isExpired || m_state != MagicState::Fly)
	{
		return;
	}

	// 魔法の当たり判定用スフィアを作成する。
	DirectX::BoundingSphere magicSphere;
	magicSphere.Center = GetPos();
	magicSphere.Radius = m_radius;

	KdCollider::SphereInfo sphereInfo(KdCollider::TypeDamage, magicSphere);

	// シーン内の敵を調べ、魔法が当たった相手にダメージを与える。
	for (const std::shared_ptr<KdGameObject>& spObj : SceneManager::Instance().GetObjList())
	{
		if (!spObj) { continue; }

		std::shared_ptr<EnemyBase> spEnemy = std::dynamic_pointer_cast<EnemyBase>(spObj);
		if (!spEnemy) { continue; }
		if (spEnemy->IsExpired()) { continue; }
		if (spEnemy == m_wpIgnoreTarget.lock()) { continue; }
		if (HasHitObject(spEnemy)) { continue; }

		std::list<KdCollider::CollisionResult> retList;
		if (spEnemy->Intersects(sphereInfo, &retList))
		{
			AddHitObject(spEnemy);

			OnBeforeDamage(spEnemy);

			// 敵に魔法のダメージ量を渡す。
			spEnemy->OnHit(m_damage);

			OnAfterDamage(spEnemy);

			if (ShouldKeepFlyingAfterHit(spEnemy))
			{
				continue;
			}

			StartHit();
			break;
		}
	}
}

void MagicBase::DrawLit()
{
	if (!m_spPoly) { return; }

	// 詠唱中は2.5DOriginalと同じくディゾルブ値を使って出現させる。
	float range = 0.05f;
	Math::Vector3 color = { 0.8f, 0.9f, 1.0f };
	if (m_state == MagicState::Chant)
	{
		KdShaderManager::Instance().m_StandardShader.SetDissolve(m_chant, &range, &color);
	}

	KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_spPoly, m_mWorld);
	KdShaderManager::Instance().m_StandardShader.SetDissolve(0.0f);
}

void MagicBase::Shot(
	const Math::Vector3& startPos,
	const Math::Vector3& dir,
	MagicType type,
	float damage,
	float speed,
	const std::shared_ptr<KdGameObject>& chantTarget,
	const std::shared_ptr<KdGameObject>& flyTarget,
	const std::shared_ptr<KdGameObject>& ignoreTarget)
{
	m_pos = startPos;
	m_dir = dir;
	m_magicType = type;
	m_damage = damage;
	m_speed = speed;
	m_state = MagicState::Chant;
	m_chant = 1.0f;
	m_frame = 0.0f;
	m_nowFrame = -1;
	m_wpChantTarget = chantTarget;
	m_wpFlyTarget = flyTarget;
	m_wpIgnoreTarget = ignoreTarget;
	m_hitObjectList.clear();

	// 詠唱開始時点の「追従対象から見た魔法の位置差」を保存する。
	if (chantTarget)
	{
		m_chantOffset = startPos - chantTarget->GetPos();
	}
	else
	{
		m_chantOffset = Math::Vector3::Zero;
	}

	if (m_dir.LengthSquared() > 0.0001f)
	{
		m_dir.Normalize();
	}

	SetupMagic();

	UpdateWorldMatrix();
}

void MagicBase::SetupMagic()
{
	if (!m_spPoly)
	{
		m_spPoly = std::make_shared<KdSquarePolygon>();
	}

	m_framePathList.clear();
}

void MagicBase::UpdateHitMagic()
{
	m_isExpired = true;
}

void MagicBase::UpdateFrameAnimation()
{
	if (m_framePathList.empty()) { return; }

	m_frame += m_frameSpeed;
	const int frameIndex = static_cast<int>(m_frame) % static_cast<int>(m_framePathList.size());
	SetFrameTexture(frameIndex);
}

void MagicBase::UpdateWorldMatrix()
{
	// 移動行列。
	Math::Matrix m_trans = Math::Matrix::CreateTranslation(m_pos);

	// 画像素材の向きに合わせるための補正回転。
	Math::Matrix m_rotX = Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(90.0f));
	Math::Matrix m_rotYBase = Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(90.0f));
	float angle = atan2f(m_dir.x, m_dir.z) + GetDirectionAngleOffset();

	Math::Matrix m_rotDir = Math::Matrix::CreateRotationY(angle);

	m_mWorld = m_rotX * m_rotYBase * m_rotDir * m_trans;
}

void MagicBase::SetFrameTexture(int frameIndex)
{
	if (!m_spPoly) { return; }
	if (m_framePathList.empty()) { return; }

	frameIndex = std::clamp(frameIndex, 0, static_cast<int>(m_framePathList.size()) - 1);
	if (frameIndex == m_nowFrame) { return; }

	m_nowFrame = frameIndex;
	m_spPoly->SetMaterial(m_framePathList[frameIndex]);
}

void MagicBase::PlayShotSound()
{
	const char* soundPath = GetShotSoundPath();
	if (!soundPath || soundPath[0] == '\0') { return; }

	// 音素材を追加したらGetShotSoundPath()にパスを入れるだけでここから再生される。
	auto sound = KdAudioManager::Instance().Play(soundPath);
	if (sound)
	{
		sound->SetVolume(0.45f);
	}
}

void MagicBase::PlayHitSound()
{
	const char* soundPath = GetHitSoundPath();
	if (!soundPath || soundPath[0] == '\0') { return; }

	// 命中音は発射音より少し大きめにすると、当たった手応えが分かりやすい。
	auto sound = KdAudioManager::Instance().Play(soundPath);
	if (sound)
	{
		sound->SetVolume(0.55f);
	}
}

bool MagicBase::HasHitObject(const std::shared_ptr<KdGameObject>& obj) const
{
	if (!obj) { return false; }

	for (const std::weak_ptr<KdGameObject>& wpHitObj : m_hitObjectList)
	{
		if (wpHitObj.lock() == obj)
		{
			return true;
		}
	}

	return false;
}

void MagicBase::AddHitObject(const std::shared_ptr<KdGameObject>& obj)
{
	if (!obj) { return; }
	if (HasHitObject(obj)) { return; }

	m_hitObjectList.push_back(obj);
}

























