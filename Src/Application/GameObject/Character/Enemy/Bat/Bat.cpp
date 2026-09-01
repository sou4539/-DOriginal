#include "Bat.h"

#include "../../Player/Player.h"
#include "../../Status/Status.h"

namespace
{
	// FPS優先のため、遠いコウモリはアニメと影の処理を止める。
	constexpr float BatAnimActiveRadius = 30.0f;
	constexpr float BatShadowDrawRadius = 22.0f;
	constexpr float BatDebugDrawRadius = 80.0f;
	constexpr float BatSearchRadius = 12.0f;
	constexpr float BatChaseRadius = 24.0f;
	constexpr float BatMoveSpeed = 0.11f;
	constexpr int BatHitFlashFrame = 5;
	constexpr int BatNearAnimInterval = 2;
	const Math::Color BatHitColor = Math::Color(2.0f, 0.05f, 0.05f, 1.0f);
	const Math::Vector3 BatHitEmissive = Math::Vector3(1.2f, 0.0f, 0.0f);

}

void Bat::Init()
{

	// コウモリモデルを読み込む。

	if (!m_spModel)
	{
		m_spModel = std::make_shared<KdModelWork>();
		m_spModel->SetModelData("Asset/Models/Objects/Character/Bat/Bat.gltf");
	}

	// Play bat flap animation.
	if (m_spModel)
	{
		m_animator.SetAnimation(m_spModel->GetAnimation("flap_loop"), true);
	}


	// Set temporary initial position.
	m_pos = { -15,3,0 };
	m_startPos = m_pos;
	SetPos(m_pos);

	// 索敵範囲をデバッグ表示するためのワイヤーを作成する。
	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

	// コウモリにダメージ判定を持たせる。
	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape
	(
		"BatDamage",
		Math::Vector3::Zero,
		m_damageRadius,
		KdCollider::TypeDamage
	);
}

void Bat::Update()
{
	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
	float distanceSqr = 0.0f;
	bool hasTargetDistance = false;
	bool isMoving = false;

	if (spTarget)
	{
		Math::Vector3 toTarget = spTarget->GetPos() - m_pos;
		distanceSqr = toTarget.LengthSquared();
		hasTargetDistance = true;
	}

	if (m_hitFlashFrame > 0)
	{
		--m_hitFlashFrame;
	}

	// デバッグ表示がONで、近くにいる時だけ索敵範囲を作る。
	if (m_pDebugWire && KdDebugWireFrame::IsEnable())
	{
		bool canDrawDebug = true;
		if (spTarget)
		{
			Math::Vector3 toTarget = m_pos - spTarget->GetPos();
			toTarget.y = 0.0f;
			canDrawDebug = toTarget.LengthSquared() <= BatDebugDrawRadius * BatDebugDrawRadius;
		}

		if (canDrawDebug)
		{
			float debugRadius = m_isChasing ? BatChaseRadius : BatSearchRadius;
			m_pDebugWire->AddDebugSphere(m_pos, debugRadius, kRedColor);
		}
	}

	if (spTarget)
	{
		// プレイヤーが村の安全地帯にいるか確認する。
		bool isTargetInSafeArea = false;
		std::shared_ptr<Player> spPlayer = std::dynamic_pointer_cast<Player>(spTarget);
		if (spPlayer)
		{
			isTargetInSafeArea = spPlayer->IsInSafeArea();
		}

		// プレイヤーまでの方向と距離を調べる。
		Math::Vector3 toTarget = spTarget->GetPos() - m_pos;
		distanceSqr = toTarget.LengthSquared();

		// プレイヤーが安全地帯に入ったら追跡をやめる。
		if (isTargetInSafeArea)
		{
			m_isChasing = false;
		}
		else
		{
			// 見つける前は小さい索敵範囲で判定する。
			if (!m_isChasing && distanceSqr <= BatSearchRadius * BatSearchRadius)
			{
				m_isChasing = true;
			}
			else if (m_isChasing && distanceSqr > BatChaseRadius * BatChaseRadius)
			{
				m_isChasing = false;
			}
		}

		// 追跡状態ならプレイヤーへ移動する。
		if (m_isChasing)
		{
			// 距離がほぼ0だと正規化できないため、離れている時だけ進む。
			if (distanceSqr > 0.0001f)
			{
				toTarget.Normalize();

				// プレイヤーより少し遅い速度で近づく。
				m_pos += toTarget * BatMoveSpeed;
				isMoving = true;

				// 移動方向に合わせてコウモリの向きを変える。
				m_angle = atan2(toTarget.x, toTarget.z);
			}
		}
		else
		{
			// プレイヤーを見失った場合や安全地帯にいる場合は初期位置へ戻る。
			Math::Vector3 toStart = m_startPos - m_pos;
			float startDistanceSqr = toStart.LengthSquared();
			float moveSpeedSqr = BatMoveSpeed * BatMoveSpeed;

			// 初期位置までの距離が1フレームの移動量以下なら到着扱いにする。
			if (startDistanceSqr <= moveSpeedSqr)
			{
				isMoving = (startDistanceSqr > 0.0001f);
				m_pos = m_startPos;
			}
			else
			{
				toStart.Normalize();
				m_pos += toStart * BatMoveSpeed;
				isMoving = true;

				// 戻る時も移動方向に向きを合わせる。
				m_angle = atan2(toStart.x, toStart.z);
			}
		}
	}

	// コウモリ全体のワールド行列を作る。

	Math::Matrix scaleMat = Math::Matrix::CreateScale(0.5);
	// Batモデルの正面方向が移動方向と逆なので、PIを足して向きを合わせる。
	Math::Matrix rotMat = Math::Matrix::CreateRotationY(m_angle + DirectX::XM_PI);
	Math::Matrix transMat = Math::Matrix::CreateTranslation(m_pos);
	m_mWorld = scaleMat * rotMat * transMat;

	const float animActiveRadiusSqr = BatAnimActiveRadius * BatAnimActiveRadius;
	const bool isNearAnimRange = (!hasTargetDistance || distanceSqr <= animActiveRadiusSqr);
	const bool shouldUpdateAnimThisFrame = isNearAnimRange && (++m_animUpdateFrame % BatNearAnimInterval == 0);

	if (m_spModel && shouldUpdateAnimThisFrame)
	{
		m_animator.AdvanceTime(m_spModel->WorkNodes(), static_cast<float>(BatNearAnimInterval));
	}
}

void Bat::DrawLit()
{
	if (!m_spModel) { return; }

	if (m_hitFlashFrame > 0)
	{
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld, BatHitColor, BatHitEmissive);
		return;
	}

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld, kWhiteColor);

}

void Bat::GenerateDepthMapFromLight()
{
	if (!m_spModel) { return; }

	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
	if (spTarget)
	{
		Math::Vector3 toTarget = m_pos - spTarget->GetPos();
		toTarget.y = 0.0f;
		if (toTarget.LengthSquared() > BatShadowDrawRadius * BatShadowDrawRadius) { return; }
	}

	// 遠いコウモリの影は見えにくいので描画せず、影用のモデル描画を減らす。
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

void Bat::DrawEffect()
{
	// Bat uses the real shadow map, so no fake flattened shadow is drawn here.
}

void Bat::OnHit()
{
	// ダメージ指定なしで呼ばれた場合は、基本ダメージを使う。
	OnHit(10.0f);
}

void Bat::OnHit(float damage)
{
	// すでに死亡処理中なら、経験値が重複しないよう何もしない。
	if (m_isExpired) { return; }

	// 0以下のダメージは無効にする。
	if (damage <= 0.0f) { return; }

	// 魔法が当たったので、受け取ったダメージ分だけHPを減らす。
	m_hp -= damage;
	m_hitFlashFrame = BatHitFlashFrame;

	// HPが0以下なら、BaseScene::PreUpdateで削除されるようにする。
	if (m_hp <= 0.0f)
	{
		m_isExpired = true;

		// コウモリを倒した報酬として、プレイヤーのStatusへ経験値を渡す。
		std::shared_ptr<Status> spStatus = m_wpStatus.lock();
		if (spStatus)
		{
			spStatus->AddExp(m_exp);
		}
	}
}









