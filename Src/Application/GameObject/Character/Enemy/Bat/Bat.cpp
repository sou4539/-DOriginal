#include "Bat.h"

#include "../../Player/Player.h"
#include "../../Status/Status.h"

namespace
{
	// 遠くのコウモリまで毎フレーム骨アニメーションを進めると、
	constexpr float BatAnimActiveRadius = 45.0f;
	constexpr int BatHitFlashFrame = 5;
}

void Bat::Init()
{

	//コウモリモデルを読み込む。

	if (!m_spModel)
	{
		m_spModel = std::make_shared<KdModelWork>();
		m_spModel->SetModelData("Asset/Models/Objects/Character/Bat/Bat.gltf");
	}


	//モデルに入っているアニメーションを名前で取得して再生する。

	if (m_spModel)
	{
		m_animator.SetAnimation(m_spModel->GetAnimation("flap_loop"), true);
	}


	//仮表示用に、プレイヤーと同じ初期座標へ配置する。

	m_pos = { -15,3,0 };
	m_startPos = m_pos;
	SetPos(m_pos);

	// 感知範囲をデバッグ表示するためのワイヤを作成する。
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

	if (spTarget)
	{
		Math::Vector3 toTarget = spTarget->GetPos() - m_pos;
		distanceSqr = toTarget.LengthSquared();
		hasTargetDistance = true;
	}

	//アニメーションを1フレーム進める。

	const float animActiveRadiusSqr = BatAnimActiveRadius * BatAnimActiveRadius;
	if (m_spModel && (!hasTargetDistance || m_isChasing || distanceSqr <= animActiveRadiusSqr))
	{
		m_animator.AdvanceTime(m_spModel->WorkNodes(), 1.0f);
	}

	if (m_hitFlashFrame > 0)
	{
		--m_hitFlashFrame;
	}

	// コウモリの索敵範囲をデバッグワイヤで表示する。
	if (m_pDebugWire)
	{
		float debugRadius = m_isChasing ? m_chaseRadius : m_searchRadius;
		m_pDebugWire->AddDebugSphere(m_pos, debugRadius, kRedColor);
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

		// 村の安全地帯に入ったら、追跡状態を解除する。
		if (isTargetInSafeArea)
		{
			m_isChasing = false;
		}
		else
		{
			// 見つかる前は小さい発見範囲で判定する。
			if (!m_isChasing && distanceSqr <= m_searchRadius * m_searchRadius)
			{
				m_isChasing = true;
			}
			else if (m_isChasing && distanceSqr > m_chaseRadius * m_chaseRadius)
			{
				m_isChasing = false;
			}
		}

		// 追跡状態ならプレイヤーへ移動する。
		if (m_isChasing)
		{
			// 距離が0に近いと正規化できないため、少し離れている時だけ動く。
			if (distanceSqr > 0.0001f)
			{
				toTarget.Normalize();

				// プレイヤーより少し遅い速度で近づく。
				m_pos += toTarget * m_moveSpeed;

				// 移動方向に合わせてコウモリの向きを変える。
				m_angle = atan2(toTarget.x, toTarget.z);
			}
		}
		else
		{
			// プレイヤーが感知スフィアの外にいる、または安全地帯にいる場合は、
			Math::Vector3 toStart = m_startPos - m_pos;
			float startDistanceSqr = toStart.LengthSquared();
			float moveSpeedSqr = m_moveSpeed * m_moveSpeed;

			// 初期位置までの距離が1フレームの移動量以下なら、
			if (startDistanceSqr <= moveSpeedSqr)
			{
				m_pos = m_startPos;
			}
			else
			{
				toStart.Normalize();
				m_pos += toStart * m_moveSpeed;

				// 戻る時も移動方向に向きを合わせる。
				m_angle = atan2(toStart.x, toStart.z);
			}
		}
	}

	//コウモリ全体のワールド行列を作る。

	Math::Matrix scaleMat = Math::Matrix::CreateScale(0.5);
	// Batモデルの正面方向が移動方向の計算と逆向きなので、
	Math::Matrix rotMat = Math::Matrix::CreateRotationY(m_angle + DirectX::XM_PI);
	Math::Matrix transMat = Math::Matrix::CreateTranslation(m_pos);
	m_mWorld = scaleMat * rotMat * transMat;
}

void Bat::DrawLit()
{
	if (!m_spModel) { return; }

	const Math::Color drawColor = (m_hitFlashFrame > 0) ? kRedColor : kWhiteColor;
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld, drawColor);

}

void Bat::OnHit()
{
	// 引数なしで呼ばれた場合は、仮の基本ダメージを使う。
	OnHit(10.0f);
}

void Bat::OnHit(float damage)
{
	// すでに死亡処理が入っている場合は、二重に経験値が入らないように何もしない。
	if (m_isExpired) { return; }

	// 0以下のダメージは無効にする。
	if (damage <= 0.0f) { return; }

	// 魔法が当たったので、受け取ったダメージ量だけHPを減らす。
	m_hp -= damage;
	m_hitFlashFrame = BatHitFlashFrame;

	// HPが0以下になったら、BaseScene::PreUpdateで削除されるようにする。
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









