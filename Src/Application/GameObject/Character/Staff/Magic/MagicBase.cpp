#include "MagicBase.h"

#include "../../../../Scene/SceneManager.h"
#include "../../Bat/Bat.h"

#include <algorithm>

namespace
{
	// 2.5DOriginalのdと同じ考え方。
	// 値が大きいほど詠唱が早く終わる。
	constexpr float MagicChantSpeed = 0.05f;

	// 画像切り替え速度。
	// 1.0なら毎フレーム切り替え、0.2なら5フレームに1回程度切り替わる。
	constexpr float IceFrameSpeed = 0.12f;
	constexpr float VoltFrameSpeed = 0.20f;
	constexpr float FireFrameSpeed = 0.25f;

	// 現在の見た目に合わせた暫定サイズ。
	// 後で実機確認しながら魔法ごとに調整する。
	constexpr float FireScale = 4.0f;
	constexpr float IceScale = 4.0f;
	constexpr float VoltScale = 4.0f;

	// 魔法弾の当たり判定半径。
	// KdSquarePolygonのSetScaleは見た目の短辺サイズを決めるため、
	// 半径は「見た目サイズの半分」を基準にする。
	// 見た目サイズを変えた時に、当たり判定も一緒に変わるようにしている。
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
		// m_pDebugWire->AddDebugSphere(m_pos, m_radius);
	}

	UpdateWorldMatrix();
}

void MagicBase::UpdateChant()
{
	// 詠唱中だけ、Shot()で受け取った対象を追いかける。
	// 今は杖を対象にしているため、杖がプレイヤーの周りを回っても魔法は杖の上に残る。
	// m_chantOffsetには「発生した瞬間の杖から見た位置差」を保存しているので、
	// 杖の少し上など、見た目用の位置を保ったまま追従できる。
	if (auto spChantTarget = m_wpChantTarget.lock())
	{
		m_pos = spChantTarget->GetPos() + m_chantOffset;
	}

	// 詠唱中は2.5DOriginalと同じく、値を1.0から0.0へ減らしていく。
	// この間は移動せず、杖の位置から発射された魔法が少し溜まって見える。
	m_chant -= m_chantSpeed;

	// 氷は「3枚目になったら発射」する仕様。
	// Ice0 → Ice1 → Ice2 と進み、Ice2が表示されたら飛行状態へ移る。
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
	// Fireは衝突用フレームを最後まで再生してから消える。
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
	// 発射音は「魔法生成時」ではなく「飛び始めた時」に鳴らすと、
	// 詠唱演出とタイミングを合わせやすい。
	// 飛び始めた後まで杖を追いかけると弾道が曲がってしまうため、
	// ここで追従対象を外し、以降はm_dir方向へまっすぐ進ませる。
	m_wpChantTarget.reset();

	// 詠唱中に敵や杖が動いた場合に備えて、
	// 発射する瞬間の「現在の魔法位置」から「現在の敵位置」へ向き直す。
	// ここではY成分も消さないため、上下方向にも飛ぶ。
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
	// ヒット音はここに集めておくと、後で魔法ごとの音差し替えが簡単になる。
	PlayHitSound();

	if (m_magicType == MagicType::Fire)
	{
		// FireはFire.pngの後半フレームを使って命中演出を再生してから消える。
		m_state = MagicState::Hit;
		m_frame = 0.0f;
		m_nowFrame = -1;
		if (m_spPoly)
		{
			m_spPoly->SetUVRect(m_fireHitFrameStart);
		}
	}
	else
	{
		// Ice / Voltはまだ専用ヒット演出がないため、命中したらすぐ消す。
		// 画像素材を追加したら、ここをHit状態へ移す形に変更できる。
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
	// TypeDamageを見ることで、敵が持っているダメージ判定に当たったかを確認する。
	DirectX::BoundingSphere magicSphere;
	magicSphere.Center = GetPos();
	magicSphere.Radius = m_radius;

	KdCollider::SphereInfo sphereInfo(KdCollider::TypeDamage, magicSphere);

	// シーン内のオブジェクトを調べ、Batに当たったらダメージを与える。
	for (const std::shared_ptr<KdGameObject>& spObj : SceneManager::Instance().GetObjList())
	{
		if (!spObj) { continue; }

		// 今は敵がBatだけなので、Batに変換できたものだけを攻撃対象にする。
		// 後でEnemyBaseを作ったら、ここをEnemyBase判定に変更する。
		std::shared_ptr<Bat> spBat = std::dynamic_pointer_cast<Bat>(spObj);
		if (!spBat) { continue; }
		if (spBat->IsExpired()) { continue; }
		if (spBat == m_wpIgnoreTarget.lock()) { continue; }
		if (HasHitObject(spBat)) { continue; }

		std::list<KdCollider::CollisionResult> retList;
		if (spBat->Intersects(sphereInfo, &retList))
		{
			AddHitObject(spBat);

			// 敵に魔法のダメージ量を渡す。
			// 命中時の音や演出開始はStartHit()側にまとめている。
			spBat->OnHit(m_damage);

			// 炎は命中した敵の周囲にもダメージを与える。
			if (m_magicType == MagicType::Fire)
			{
				ApplyFireExplosion(spBat);
			}

			// 雷は基本性能として連鎖する。
			// 当たった敵の近くに別の敵がいれば、同じVolt画像の弾を追加で飛ばす。
			if (m_magicType == MagicType::Volt)
			{
				CreateVoltChain(spBat);
			}

			if (m_magicType == MagicType::Ice)
			{
				// 通常の氷弾は、最初に敵へ触れた時だけ派生弾を出す。
				// 派生弾はm_isIceSplitShotがtrueなので、ここからさらに増えることはない。
				if (!m_isIceSplitShot && !m_hasCreatedIceSplit)
				{
					CreateIceSplit(spBat);
					m_hasCreatedIceSplit = true;
				}

				// 氷は貫通魔法。
				// 残り貫通数がある間は消えず、次の敵へ当たれるように飛び続ける。
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
	// UpdateChant()では、この差分を使って杖の移動分だけ魔法を動かす。
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
	// 詠唱を挟まずにすぐ飛ばす。
	// これにより「当たった場所から次の弾が出る」見た目になる。
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
	// m_rotDirは魔法の進行方向に合わせてY軸回転する。
	Math::Matrix m_rotX = Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(90.0f));
	Math::Matrix m_rotYBase = Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(90.0f));
	float angle = atan2f(m_dir.x, m_dir.z);

	// Fire.pngは画像の先端向きがIce/Voltと逆になっているため、炎だけ180度補正する。
	// 実際の移動方向m_dirは変えず、見た目の向きだけを反転させる。
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
		// 例："Asset/Audio/Magic/FireShot.wav"
		return "";
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
		// 例："Asset/Audio/Magic/FireHit.wav"
		return "";
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

void MagicBase::CreateVoltChain(const std::shared_ptr<Bat>& hitBat)
{
	if (m_magicType != MagicType::Volt) { return; }
	if (m_voltChainCount <= 0) { return; }
	if (!hitBat) { return; }

	std::shared_ptr<Bat> spNextTarget = SearchVoltChainTarget(hitBat);
	if (!spNextTarget) { return; }

	Math::Vector3 startPos = hitBat->GetPos();
	Math::Vector3 dir = spNextTarget->GetPos() - startPos;
	if (dir.LengthSquared() <= 0.0001f) { return; }
	dir.Normalize();

	// 連鎖元のコウモリに即再ヒットしないように、少しだけ次の敵方向へずらして生成する。
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
		hitBat,
		true,
		m_fireExplosionRadius,
		m_icePierceCount,
		m_iceSplitCount,
		false
	);

	SceneManager::Instance().AddObject(spChainMagic);
}

void MagicBase::CreateIceSplit(const std::shared_ptr<Bat>& hitBat)
{
	if (m_magicType != MagicType::Ice) { return; }
	if (m_isIceSplitShot) { return; }
	if (m_iceSplitCount <= 0) { return; }
	if (!hitBat) { return; }
	if (m_dir.LengthSquared() <= 0.0001f) { return; }

	// 派生弾は、当たった敵の位置から現在の進行方向へ出す。
	// 開始位置を少し前にずらし、命中した敵へ即再ヒットしないようにする。
	const float centerOffset = static_cast<float>(m_iceSplitCount - 1) * 0.5f;

	for (int i = 0; i < m_iceSplitCount; ++i)
	{
		const float angle = (static_cast<float>(i) - centerOffset) * IceSplitSpreadAngle;
		const Math::Vector3 splitDir = RotateDirY(m_dir, angle);
		Math::Vector3 startPos = hitBat->GetPos() + splitDir * (m_radius + 0.2f);

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
			hitBat,
			false,
			m_fireExplosionRadius,
			1,
			0,
			true
		);

		SceneManager::Instance().AddObject(spSplitMagic);
	}
}

std::shared_ptr<Bat> MagicBase::SearchVoltChainTarget(const std::shared_ptr<Bat>& hitBat)
{
	if (!hitBat) { return nullptr; }

	std::shared_ptr<Bat> spTarget = nullptr;
	const Math::Vector3 hitPos = hitBat->GetPos();
	float minDistanceSqr = m_voltChainRadius * m_voltChainRadius;

	for (const std::shared_ptr<KdGameObject>& spObj : SceneManager::Instance().GetObjList())
	{
		if (!spObj) { continue; }

		std::shared_ptr<Bat> spBat = std::dynamic_pointer_cast<Bat>(spObj);
		if (!spBat) { continue; }
		if (spBat == hitBat) { continue; }
		if (spBat == m_wpIgnoreTarget.lock()) { continue; }
		if (spBat->IsExpired()) { continue; }

		const Math::Vector3 toBat = spBat->GetPos() - hitPos;
		const float distanceSqr = toBat.LengthSquared();
		if (distanceSqr < minDistanceSqr)
		{
			minDistanceSqr = distanceSqr;
			spTarget = spBat;
		}
	}

	return spTarget;
}

void MagicBase::ApplyFireExplosion(const std::shared_ptr<Bat>& hitBat)
{
	if (m_magicType != MagicType::Fire) { return; }
	if (!hitBat) { return; }
	if (m_fireExplosionRadius <= 0.0f) { return; }

	const Math::Vector3 explosionCenter = hitBat->GetPos();
	const float explosionRadiusSqr = m_fireExplosionRadius * m_fireExplosionRadius;

	for (const std::shared_ptr<KdGameObject>& spObj : SceneManager::Instance().GetObjList())
	{
		if (!spObj) { continue; }

		std::shared_ptr<Bat> spBat = std::dynamic_pointer_cast<Bat>(spObj);
		if (!spBat) { continue; }
		if (spBat == hitBat) { continue; }
		if (spBat->IsExpired()) { continue; }

		const Math::Vector3 toBat = spBat->GetPos() - explosionCenter;
		if (toBat.LengthSquared() <= explosionRadiusSqr)
		{
			spBat->OnHit(m_damage);
			AddHitObject(spBat);
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

























