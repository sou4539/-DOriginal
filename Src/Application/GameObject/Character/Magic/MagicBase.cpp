#include "MagicBase.h"

#include "../../../Scene/SceneManager.h"
#include "../Enemy/EnemyBase.h"

#include <algorithm>

namespace
{
	// 2.5DOriginalのdと同じ考え方。
	constexpr float MagicChantSpeed = 0.05f;
	constexpr float MagicHitCheckMargin = 3.0f;
}

void MagicBase::Init()
{
	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

	m_pos = {};
	m_dir = {};
	m_state = MagicState::Chant;
	m_damage = 0.0f;
	m_speed = 0.0f;
	m_lifeFrames = 0.0f;
	m_radius = 0.0f;
	m_chant = 1.0f;
	m_chantSpeed = MagicChantSpeed;
	m_framePaths.clear();
	m_animFrame = 0.0f;
	m_frameSpeed = 0.0f;
	m_frameIndex = -1;
	m_wpChantTarget.reset();
	m_chantOffset = Math::Vector3::Zero;
	m_wpFlyTarget.reset();
	m_wpIgnoreTarget.reset();
	m_hitObjects.clear();

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
	m_lifeFrames -= 1.0f;
	if (m_lifeFrames <= 0.0f)
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

	// プレイヤー周辺の敵だけを調べ、全オブジェクト検索を避ける。
	for (const std::weak_ptr<EnemyBase>& wpEnemy : SceneManager::Instance().GetActiveEnemies())
	{
		std::shared_ptr<EnemyBase> spEnemy = wpEnemy.lock();
		if (!spEnemy) { continue; }
		if (spEnemy->IsExpired()) { continue; }
		if (spEnemy == m_wpIgnoreTarget.lock()) { continue; }
		if (HasHitObject(spEnemy)) { continue; }

		const float checkRadius = m_radius + MagicHitCheckMargin;
		Math::Vector3 toEnemy = spEnemy->GetPos() - m_pos;
		if (toEnemy.LengthSquared() > checkRadius * checkRadius) { continue; }

		std::list<KdCollider::CollisionResult> hits;
		if (spEnemy->Intersects(sphereInfo, &hits))
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

	KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_spPoly, m_mWorld, kWhiteColor, GetEmissiveColor());
	KdShaderManager::Instance().m_StandardShader.SetDissolve(0.0f);
}

void MagicBase::Shot(const MagicShotParams& params)
{
	// 共通データを弾へ保存する。paramsへの参照そのものは保持しない。
	m_pos = params.startPos;					// 発射位置
	m_dir = params.dir;							// 発射方向
	m_damage = params.damage;					// ダメージ倍率
	m_speed = params.speed;						// 1フレームの移動量
	m_state = MagicState::Chant;				// 初期状態は詠唱中
	m_chant = 1.0f;								// 詠唱中は1.0から0.0へ減らす
	m_animFrame = 0.0f;							// 画像アニメーションのフレーム位置
	m_frameIndex = -1;							// 画像アニメーションのフレーム番号
	m_wpChantTarget = params.chantTarget;		// 詠唱中の追従対象
	m_wpFlyTarget = params.flyTarget;			// 発射時の照準対象
	m_wpIgnoreTarget = params.ignoreTarget;		// 当たり判定から除外する対象
	m_hitObjects.clear();						// すでに当たった敵の記録をクリアする

	// 杖との位置差を保存し、詠唱中も杖の少し上に表示する。
	m_chantOffset = params.chantTarget
		? params.startPos - params.chantTarget->GetPos()
		: Math::Vector3::Zero;
	if (m_dir.LengthSquared() > 0.0001f)
	{
		// 正規化
		m_dir.Normalize();
	}

	// 専用設定を先に済ませる。雷は連鎖弾かどうかで画像が変わるため順序が重要。
	const bool skipChant = ConfigureShot(params);
	SetupMagic();

	// 画像・寿命を準備してから即時発射する。通常弾は詠唱状態のまま待つ。
	if (skipChant)
	{
		StartFly();
	}
	UpdateWorldMatrix();
}

void MagicBase::SetupMagic()
{
	if (!m_spPoly)
	{
		m_spPoly = std::make_shared<KdSquarePolygon>();
	}

	m_framePaths.clear();
}

void MagicBase::UpdateHitMagic()
{
	m_isExpired = true;
}

void MagicBase::UpdateFrameAnimation()
{
	if (m_framePaths.empty()) { return; }

	m_animFrame += m_frameSpeed;
	const int frameIndex = static_cast<int>(m_animFrame) % static_cast<int>(m_framePaths.size());
	SetFrameTexture(frameIndex);
}

void MagicBase::UpdateWorldMatrix()
{
	// 移動行列。
	Math::Matrix translation = Math::Matrix::CreateTranslation(m_pos);

	// 画像素材の向きに合わせるための補正回転。
	Math::Matrix tilt = Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(90.0f));
	Math::Matrix baseRotation = Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(90.0f));
	float angle = atan2f(m_dir.x, m_dir.z) + GetDirectionAngleOffset();

	Math::Matrix aimRotation = Math::Matrix::CreateRotationY(angle);

	m_mWorld = tilt * baseRotation * aimRotation * translation;
}

void MagicBase::SetFrameTexture(int frameIndex)
{
	if (!m_spPoly) { return; }
	if (m_framePaths.empty()) { return; }

	frameIndex = std::clamp(frameIndex, 0, static_cast<int>(m_framePaths.size()) - 1);
	if (frameIndex == m_frameIndex) { return; }

	m_frameIndex = frameIndex;
	m_spPoly->SetMaterial(m_framePaths[frameIndex]);
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

	for (const std::weak_ptr<KdGameObject>& wpHitObj : m_hitObjects)
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

	m_hitObjects.push_back(obj);
}

























