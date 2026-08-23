#include "MagicBase.h"

#include "../../../../Scene/SceneManager.h"
#include "../../Enemy/EnemyBase.h"

#include <algorithm>

namespace
{
	// 2.5DOriginalのdと同じ考え方。
	constexpr float MagicChantSpeed = 0.05f;

	// 画像切り替え速度。
	constexpr float IceFrameSpeed = 0.12f;
	constexpr float VoltFrameSpeed = 0.20f;
	constexpr float FireFrameSpeed = 0.25f;

	// 現在の見た目に合わせた暫定サイズ。
	constexpr float FireScale = 4.0f;
	constexpr float IceScale = 4.0f;
	constexpr float VoltScale = 4.0f;
	constexpr float FireBaseExplosionRadius = 3.0f;

	// 魔法弾の当たり判定半径。
	constexpr float FireHitRadius = FireScale * 0.5f;
	constexpr float IceHitRadius = IceScale * 0.5f;
	constexpr float VoltHitRadius = VoltScale * 0.5f;

	// 氷の派生弾を左右に広げる角度。
	constexpr float IceSplitSpreadAngle = DirectX::XMConvertToRadians(30.0f);

	Math::Vector3 RotateDirY(const Math::Vector3& dir, float angle)
	{
		const float cosAngle = cosf(angle);
		const float sinAngle = sinf(angle);

		Math::Vector3 ret;
		ret.x = dir.x * cosAngle + dir.z * sinAngle;
		ret.y = dir.y;
		ret.z = -dir.x * sinAngle + dir.z * cosAngle;

		if (ret.LengthSquared() > 0.0001f)
		{
			ret.Normalize();
		}

		return ret;
	}
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
	m_voltChainCount = 0;
	m_isChainShot = false;
	m_fireExplosionRadius = 3.0f;
	m_icePierceCount = 1;
	m_iceSplitCount = 1;
	m_isIceSplitShot = false;
	m_hasCreatedIceSplit = false;
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

	// 氷は「3枚目になったら発射」する仕様。
	if (m_magicType == MagicType::Ice)
	{
		UpdateFrameAnimation();

		if (m_nowFrame >= 2)
		{
			StartFly();
		}
		return;
	}

	// 炎はスプライトシートの移動用先頭フレームを詠唱中にも表示する。
	if (m_magicType == MagicType::Fire && m_spPoly)
	{
		m_spPoly->SetUVRect(m_fireFlyFrameStart);
	}

	// 雷は詠唱中も今のフレームを軽くアニメーションさせる。
	if (m_magicType == MagicType::Volt)
	{
		UpdateFrameAnimation();
	}

	if (m_chant <= 0.0f)
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

	// 雷は飛行中に画像を順番に切り替える。
	if (m_magicType == MagicType::Volt)
	{
		UpdateFrameAnimation();
	}

	// 炎はFire.pngの移動用フレームをループさせる。
	if (m_magicType == MagicType::Fire && m_spPoly)
	{
		m_frame += m_frameSpeed;
		const int frameCount = m_fireFlyFrameEnd - m_fireFlyFrameStart + 1;
		const int frameIndex = m_fireFlyFrameStart + (static_cast<int>(m_frame) % frameCount);

		if (frameIndex != m_nowFrame)
		{
			m_nowFrame = frameIndex;
			m_spPoly->SetUVRect(frameIndex);
		}
	}
}

void MagicBase::UpdateHit()
{
	// 現状、命中演出を持っているのはFire.pngのみ。
	if (m_magicType != MagicType::Fire || !m_spPoly)
	{
		m_isExpired = true;
		return;
	}

	m_frame += m_frameSpeed;
	const int frameIndex = m_fireHitFrameStart + static_cast<int>(m_frame);

	if (frameIndex > m_fireHitFrameEnd)
	{
		m_isExpired = true;
		return;
	}

	if (frameIndex != m_nowFrame)
	{
		m_nowFrame = frameIndex;
		m_spPoly->SetUVRect(frameIndex);
	}
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

	if (m_magicType == MagicType::Fire)
	{
		// FireはFire.pngの後半フレームを使って命中演出を再生してから消える。
		m_state = MagicState::Hit;
		m_frame = 0.0f;
		m_nowFrame = -1;
		if (m_spPoly)
		{
			// 炎の強化で広がった爆発範囲に合わせて、命中演出だけ大きくする。
			const float addScale = std::max(m_fireExplosionRadius - FireBaseExplosionRadius, 0.0f);
			m_spPoly->SetScale(FireScale + addScale);
			m_spPoly->SetUVRect(m_fireHitFrameStart);
		}
	}
	else
	{
		// Ice / Voltはまだ専用ヒット演出がないため、命中したらすぐ消す。
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

			// 敵に魔法のダメージ量を渡す。
			spEnemy->OnHit(m_damage);

			// 炎は命中した敵の周囲にもダメージを与える。
			if (m_magicType == MagicType::Fire)
			{
				ApplyFireExplosion(spEnemy);
			}

			// 雷は基本性能として連鎖する。
			if (m_magicType == MagicType::Volt)
			{
				CreateVoltChain(spEnemy);
			}

			if (m_magicType == MagicType::Ice)
			{
				// 通常の氷弾は、最初に敵へ触れた時だけ派生弾を出す。
				if (!m_isIceSplitShot && !m_hasCreatedIceSplit)
				{
					CreateIceSplit(spEnemy);
					m_hasCreatedIceSplit = true;
				}

				// 氷は貫通魔法。
				m_icePierceCount--;
				if (m_icePierceCount > 0)
				{
					continue;
				}
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
	int voltChainCount,
	const std::shared_ptr<KdGameObject>& ignoreTarget,
	bool isChainShot,
	float fireExplosionRadius,
	int icePierceCount,
	int iceSplitCount,
	bool isIceSplitShot)
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
	m_voltChainCount = voltChainCount;
	m_isChainShot = isChainShot;
	m_fireExplosionRadius = fireExplosionRadius;
	m_icePierceCount = icePierceCount;
	m_iceSplitCount = iceSplitCount;
	m_isIceSplitShot = isIceSplitShot;
	m_hasCreatedIceSplit = false;
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

	// 連鎖で作られた雷と、命中後に出る氷の派生弾は、
	if (isChainShot || isIceSplitShot)
	{
		m_chant = 0.0f;
		m_state = MagicState::Fly;
		m_wpChantTarget.reset();
		PlayShotSound();
	}

	UpdateWorldMatrix();
}

void MagicBase::SetupMagic()
{
	if (!m_spPoly)
	{
		m_spPoly = std::make_shared<KdSquarePolygon>();
	}

	m_framePathList.clear();

	switch (m_magicType)
	{
	case MagicType::Fire:
		m_lifeTime = 90.0f;
		m_radius = FireHitRadius;
		m_frameSpeed = FireFrameSpeed;
		m_spPoly->SetMaterial("Asset/Textures/Magic/Fire/Fire.png");
		m_spPoly->SetSplit(11, 1);
		m_spPoly->SetUVRect(m_fireFlyFrameStart);
		m_spPoly->SetScale(FireScale);
		m_nowFrame = m_fireFlyFrameStart;
		break;
	case MagicType::Ice:
		m_lifeTime = 120.0f;
		m_radius = IceHitRadius;
		m_frameSpeed = IceFrameSpeed;
		m_framePathList =
		{
			"Asset/Textures/Magic/Ice/Ice0.png",
			"Asset/Textures/Magic/Ice/Ice1.png",
			"Asset/Textures/Magic/Ice/Ice2.png"
		};
		SetFrameTexture(0);
		m_spPoly->SetScale(IceScale);
		break;
	case MagicType::Volt:
		m_lifeTime = 60.0f;
		m_radius = VoltHitRadius;
		m_frameSpeed = VoltFrameSpeed;
		if (m_isChainShot)
		{
			// 連鎖時は、敵から敵へ走る線のようなVolt画像を使う。
			m_framePathList =
			{
				"Asset/Textures/Magic/Volt/Volt0.png",
				"Asset/Textures/Magic/Volt/Volt1.png",
				"Asset/Textures/Magic/Volt/Volt2.png",
				"Asset/Textures/Magic/Volt/Volt3.png"
			};
		}
		else
		{
			// 通常発射時は、新しく追加したLightning画像をアニメーションさせながら飛ばす。
			m_framePathList =
			{
				"Asset/Textures/Magic/Volt/Lightning0.png",
				"Asset/Textures/Magic/Volt/Lightning1.png",
				"Asset/Textures/Magic/Volt/Lightning2.png",
				"Asset/Textures/Magic/Volt/Lightning3.png",
				"Asset/Textures/Magic/Volt/Lightning4.png",
				"Asset/Textures/Magic/Volt/Lightning5.png",
				"Asset/Textures/Magic/Volt/Lightning6.png",
				"Asset/Textures/Magic/Volt/Lightning7.png"
			};
		}
		SetFrameTexture(0);
		m_spPoly->SetScale(VoltScale);
		break;
	default:
		m_isExpired = true;
		break;
	}
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
	float angle = atan2f(m_dir.x, m_dir.z);

	// Fire.pngは画像の先端向きがIce/Voltと逆になっているため、炎だけ180度補正する。
	if (m_magicType == MagicType::Fire)
	{
		angle += DirectX::XM_PI;
	}

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

const char* MagicBase::GetShotSoundPath() const
{
	switch (m_magicType)
	{
	case MagicType::Fire:
		return "Asset/Sounds/Magic/shot.wav";
	case MagicType::Ice:
		// 例："Asset/Audio/Magic/IceShot.wav"
		return "";
	case MagicType::Volt:
		// 例："Asset/Audio/Magic/VoltShot.wav"
		return "";
	default:
		return "";
	}
}

const char* MagicBase::GetHitSoundPath() const
{
	switch (m_magicType)
	{
	case MagicType::Fire:
		return "Asset/Sounds/Magic/explosion.wav";
	case MagicType::Ice:
		// 例："Asset/Audio/Magic/IceHit.wav"
		return "";
	case MagicType::Volt:
		// 例："Asset/Audio/Magic/VoltHit.wav"
		return "";
	default:
		return "";
	}
}

void MagicBase::CreateVoltChain(const std::shared_ptr<EnemyBase>& hitEnemy)
{
	if (m_magicType != MagicType::Volt) { return; }
	if (m_voltChainCount <= 0) { return; }
	if (!hitEnemy) { return; }

	std::shared_ptr<EnemyBase> spNextTarget = SearchVoltChainTarget(hitEnemy);
	if (!spNextTarget) { return; }

	Math::Vector3 startPos = hitEnemy->GetPos();
	Math::Vector3 dir = spNextTarget->GetPos() - startPos;
	if (dir.LengthSquared() <= 0.0001f) { return; }
	dir.Normalize();

	// 連鎖元の敵に即再ヒットしないように、少しだけ次の敵方向へずらして生成する。
	startPos += dir * (m_radius + 0.2f);

	std::shared_ptr<MagicBase> spChainMagic = std::make_shared<MagicBase>();
	spChainMagic->Shot
	(
		startPos,
		dir,
		MagicType::Volt,
		m_damage,
		m_speed,
		nullptr,
		spNextTarget,
		m_voltChainCount - 1,
		hitEnemy,
		true,
		m_fireExplosionRadius,
		m_icePierceCount,
		m_iceSplitCount,
		false
	);

	SceneManager::Instance().AddObject(spChainMagic);
}

void MagicBase::CreateIceSplit(const std::shared_ptr<EnemyBase>& hitEnemy)
{
	if (m_magicType != MagicType::Ice) { return; }
	if (m_isIceSplitShot) { return; }
	if (m_iceSplitCount <= 0) { return; }
	if (!hitEnemy) { return; }
	if (m_dir.LengthSquared() <= 0.0001f) { return; }

	// 派生弾は、当たった敵の位置から現在の進行方向へ出す。
	const float centerOffset = static_cast<float>(m_iceSplitCount - 1) * 0.5f;

	for (int i = 0; i < m_iceSplitCount; ++i)
	{
		const float angle = (static_cast<float>(i) - centerOffset) * IceSplitSpreadAngle;
		const Math::Vector3 splitDir = RotateDirY(m_dir, angle);
		Math::Vector3 startPos = hitEnemy->GetPos() + splitDir * (m_radius + 0.2f);

		std::shared_ptr<MagicBase> spSplitMagic = std::make_shared<MagicBase>();
		spSplitMagic->Shot
		(
			startPos,
			splitDir,
			MagicType::Ice,
			m_damage * 0.5f,
			m_speed,
			nullptr,
			nullptr,
			0,
			hitEnemy,
			false,
			m_fireExplosionRadius,
			1,
			0,
			true
		);

		SceneManager::Instance().AddObject(spSplitMagic);
	}
}

std::shared_ptr<EnemyBase> MagicBase::SearchVoltChainTarget(const std::shared_ptr<EnemyBase>& hitEnemy)
{
	if (!hitEnemy) { return nullptr; }

	std::shared_ptr<EnemyBase> spTarget = nullptr;
	const Math::Vector3 hitPos = hitEnemy->GetPos();
	float minDistanceSqr = m_voltChainRadius * m_voltChainRadius;

	for (const std::shared_ptr<KdGameObject>& spObj : SceneManager::Instance().GetObjList())
	{
		if (!spObj) { continue; }

		std::shared_ptr<EnemyBase> spEnemy = std::dynamic_pointer_cast<EnemyBase>(spObj);
		if (!spEnemy) { continue; }
		if (spEnemy == hitEnemy) { continue; }
		if (spEnemy == m_wpIgnoreTarget.lock()) { continue; }
		if (spEnemy->IsExpired()) { continue; }

		const Math::Vector3 toEnemy = spEnemy->GetPos() - hitPos;
		const float distanceSqr = toEnemy.LengthSquared();
		if (distanceSqr < minDistanceSqr)
		{
			minDistanceSqr = distanceSqr;
			spTarget = spEnemy;
		}
	}

	return spTarget;
}

void MagicBase::ApplyFireExplosion(const std::shared_ptr<EnemyBase>& hitEnemy)
{
	if (m_magicType != MagicType::Fire) { return; }
	if (!hitEnemy) { return; }
	if (m_fireExplosionRadius <= 0.0f) { return; }

	const Math::Vector3 explosionCenter = hitEnemy->GetPos();
	const float explosionRadiusSqr = m_fireExplosionRadius * m_fireExplosionRadius;

	for (const std::shared_ptr<KdGameObject>& spObj : SceneManager::Instance().GetObjList())
	{
		if (!spObj) { continue; }

		std::shared_ptr<EnemyBase> spEnemy = std::dynamic_pointer_cast<EnemyBase>(spObj);
		if (!spEnemy) { continue; }
		if (spEnemy == hitEnemy) { continue; }
		if (spEnemy->IsExpired()) { continue; }

		const Math::Vector3 toEnemy = spEnemy->GetPos() - explosionCenter;
		if (toEnemy.LengthSquared() <= explosionRadiusSqr)
		{
			spEnemy->OnHit(m_damage);
			AddHitObject(spEnemy);
		}
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

























