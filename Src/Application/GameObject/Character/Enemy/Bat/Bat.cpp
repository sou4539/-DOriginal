#include "Bat.h"

#include "../../Player/Player.h"
#include "../../Status/Status.h"

namespace
{
	// FPS優先のため、遠いコウモリは影の処理を止める。
	constexpr float BatShadowDrawRadius = 22.0f;
	constexpr float BatDebugDrawRadius = 80.0f;
	constexpr float BatSearchRadius = 12.0f;
	constexpr float BatChaseRadius = 24.0f;
	constexpr float BatMoveSpeed = 0.11f;
	constexpr int BatHitFlashFrame = 5;
	constexpr int BatNearAnimInterval = 2;
	const Math::Vector3 BatHitRimColor = { 3.0f, 0.1f, 0.1f };

}

std::shared_ptr<KdModelWork> Bat::s_spSharedModel;
KdAnimator Bat::s_sharedAnimator;
int Bat::s_sharedAnimUpdateFrame = 0;

void Bat::UpdateSharedAnimation()
{
	if (!s_spSharedModel) { return; }

	++s_sharedAnimUpdateFrame;
	if (s_sharedAnimUpdateFrame % BatNearAnimInterval != 0) { return; }

	s_sharedAnimator.AdvanceTime(
		s_spSharedModel->WorkNodes(),
		static_cast<float>(BatNearAnimInterval));
}

void Bat::Init()
{
	// 同じモデルとボーン姿勢を全蝙蝠で共有し、一括描画できるようにする。
	if (!s_spSharedModel)
	{
		s_spSharedModel = std::make_shared<KdModelWork>();
		s_spSharedModel->SetModelData("Asset/Models/Objects/Character/Bat/Bat.gltf");
		s_sharedAnimator.SetAnimation(s_spSharedModel->GetAnimation("flap_loop"), true);
	}

	m_spModel = s_spSharedModel;

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
	// ディゾルブ処理
	if (m_hp <= 0.0f)
	{
		d = std::min(d + 0.01f, 1.0f);

		if (d >= 1.0f)
		{
			m_isExpired = true;
		}

		return; // 移動処理を実行しない
	}

	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();

	if (m_hitFlashFrame > 0)
	{
		--m_hitFlashFrame;
	}

	// デバッグ表示がONで、近くにいる時だけ索敵範囲を作る。
	if (m_pDebugWire && KdDebugWireFrame::IsEnable() && !AlwaysChases())
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
		const float distSq = toTarget.LengthSquared();

		// プレイヤーが安全地帯に入ったら追跡をやめる。
		if (AlwaysChases())
		{
			m_isChasing = true;
		}
		else if (isTargetInSafeArea)
		{
			m_isChasing = false;
		}
		else
		{
			// 見つける前は小さい索敵範囲で判定する。
			if (!m_isChasing && distSq <= BatSearchRadius * BatSearchRadius)
			{
				m_isChasing = true;
			}
			else if (m_isChasing && distSq > BatChaseRadius * BatChaseRadius)
			{
				m_isChasing = false;
			}
		}

		// 追跡状態ならプレイヤーへ移動する。
		if (m_isChasing)
		{
			// 距離がほぼ0だと正規化できないため、離れている時だけ進む。
			if (distSq > 0.0001f)
			{
				toTarget.Normalize();

				// プレイヤーより少し遅い速度で近づく。
				m_pos += toTarget * BatMoveSpeed;

				// 移動方向に合わせてコウモリの向きを変える。
				m_angle = atan2(toTarget.x, toTarget.z);
			}
		}
		else
		{
			// プレイヤーを見失った場合や安全地帯にいる場合は初期位置へ戻る。
			Math::Vector3 toStart = m_startPos - m_pos;
			float startDistSq = toStart.LengthSquared();
			float moveSpeedSq = BatMoveSpeed * BatMoveSpeed;

			// 初期位置までの距離が1フレームの移動量以下なら到着扱いにする。
			if (startDistSq <= moveSpeedSq)
			{
				m_pos = m_startPos;
			}
			else
			{
				toStart.Normalize();
				m_pos += toStart * BatMoveSpeed;

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
}

void Bat::DrawLit()
{
	if (!m_spModel) { return; }

	if (m_hitFlashFrame > 0)
	{
		// モデル本来の色を残し、被弾した5フレームだけ輪郭を赤く光らせる。
		// 一括描画なので、効果はシェーダーへ直接設定せず登録情報に保存する。
		KdModelVisualEffects effects;
		effects.RimLight = true;
		effects.RimColor = BatHitRimColor;
		effects.RimPower = 2.0f;
		KdModelInstanceBatcher::Instance().SubmitLit(
			m_spModel, m_mWorld, kWhiteColor, Math::Vector3::Zero, d, effects);
		return;
	}
	float range = 0.05;
	Math::Vector3 color = { 1,0.3,0.3 };
	KdShaderManager::Instance().m_StandardShader.SetDissolve(d, &range, &color);

	KdModelInstanceBatcher::Instance().SubmitLit(
		m_spModel, m_mWorld,
		kWhiteColor, Math::Vector3::Zero, d);

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

	float range = 0.05;
	Math::Vector3 color = { 1,0.3,0.3 };
	KdShaderManager::Instance().m_StandardShader.SetDissolve(d, &range, &color);

	// 遠いコウモリの影は見えにくいので描画せず、影用のモデル描画を減らす。
	KdModelInstanceBatcher::Instance().SubmitDepth(m_spModel, m_mWorld);
}

void Bat::DrawEffect()
{
}

void Bat::OnHit()
{
	// ダメージ指定なしで呼ばれた場合は、基本ダメージを使う。
	OnHit(10.0f);
}

void Bat::OnHit(float damage)
{
	// すでに死亡処理中なら、経験値が重複しないよう何もしない。
	if (!CanBeTargeted()) { return; }

	// 0以下のダメージは無効にする。
	if (damage <= 0.0f) { return; }

	// 魔法が当たったので、受け取ったダメージ分だけHPを減らす。
	m_hp -= damage;
	m_hitFlashFrame = BatHitFlashFrame;

	// HPが0以下なら、BaseScene::PreUpdateで削除されるようにする。
	if (m_hp <= 0.0f)
	{
		m_isChasing = false;
		m_hitFlashFrame = 0;

		if (m_pCollider)
		{
			m_pCollider->SetEnableAll(false);
		}

		// コウモリを倒した報酬として、プレイヤーのStatusへ経験値を渡す。
		std::shared_ptr<Status> spStatus = m_wpStatus.lock();
		if (spStatus)
		{
			// 経験値を加算する。
			spStatus->AddExp(m_exp);

			// コウモリを倒した数を加算する。
			if (CountsAsGrassBat()) { spStatus->AddKillBatCount(); }
		}
		// 死亡状態に入った後に一度だけ通知する。報酬は強化前の値で渡す。
		OnDefeated();
	}
}

void Bat::SetCombatStats(float maxHp, float experience)
{
	if (!CanBeTargeted()) { return; }
	const float healthRate = m_hp / m_maxHp;
	m_maxHp = maxHp;
	m_hp = maxHp * healthRate;
	m_exp = experience;
}
