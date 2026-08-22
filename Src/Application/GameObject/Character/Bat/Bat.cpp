#include "Bat.h"

#include "../Player/Player.h"
#include "../Status/Status.h"

namespace
{
	// 遠くのコウモリまで毎フレーム骨アニメーションを進めると、
	// コウモリ数が増えた時に実行時の重さへ直結する。
	// 近い敵と追跡中の敵だけ通常通り更新し、遠くの敵は見た目より軽さを優先する。
	constexpr float BatAnimActiveRadius = 45.0f;
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
	//第2引数をtrueにすると、アニメーションが最後まで行った後にループする。

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
	// localPosはコウモリ自身から見た判定中心なので、ここではモデル中心の0に置く。
	// この判定は、後でプレイヤー側や攻撃側がTypeDamageを調べる時に使う。
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
	//WorkNodes()でモデル内部のノード行列を編集できる状態にする。
	//AdvanceTime()が現在のアニメーション時間に合わせて、羽などのノードを動かす。

	const float animActiveRadiusSqr = BatAnimActiveRadius * BatAnimActiveRadius;
	if (m_spModel && (!hasTargetDistance || m_isChasing || distanceSqr <= animActiveRadiusSqr))
	{
		m_animator.AdvanceTime(m_spModel->WorkNodes(), 1.0f);
	}

	// コウモリの索敵範囲をデバッグワイヤで表示する。
	// 見つかる前は発見範囲、見つかった後は追跡継続範囲を表示する。
	if (m_pDebugWire)
	{
		float debugRadius = m_isChasing ? m_chaseRadius : m_searchRadius;
		m_pDebugWire->AddDebugSphere(m_pos, debugRadius, kRedColor);
	}

	if (spTarget)
	{
		// プレイヤーが村の安全地帯にいるか確認する。
		// 安全地帯にいる場合は、距離に関係なく追跡せず初期位置へ戻る。
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
		// これにより、安全地帯から出た後はもう一度発見範囲に入るまで追いかけない。
		if (isTargetInSafeArea)
		{
			m_isChasing = false;
		}
		else
		{
			// 見つかる前は小さい発見範囲で判定する。
			// 一度見つけた後はm_isChasingをtrueにして、広い追跡範囲で判定する。
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
		// 一度見つかった後はm_chaseRadiusを抜けるまで追跡が続く。
		if (m_isChasing)
		{
			// 距離が0に近いと正規化できないため、少し離れている時だけ動く。
			if (distanceSqr > 0.0001f)
			{
				toTarget.Normalize();

				// プレイヤーより少し遅い速度で近づく。
				// Playerの移動速度は0.1fなので、それより少し遅い速度にする。
				m_pos += toTarget * m_moveSpeed;

				// 移動方向に合わせてコウモリの向きを変える。
				m_angle = atan2(toTarget.x, toTarget.z);
			}
		}
		else
		{
			// プレイヤーが感知スフィアの外にいる、または安全地帯にいる場合は、
			// コウモリを最初に配置した座標へ少しずつ戻す。
			Math::Vector3 toStart = m_startPos - m_pos;
			float startDistanceSqr = toStart.LengthSquared();
			float moveSpeedSqr = m_moveSpeed * m_moveSpeed;

			// 初期位置までの距離が1フレームの移動量以下なら、
			// 行き過ぎないように初期位置で止める。
			// ここから先は移動せず、羽ばたきアニメーションだけ再生する。
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
	//アニメーションはモデル内部を動かす処理で、
	//ここではコウモリ全体をどこに配置するかを決めている。

	Math::Matrix scaleMat = Math::Matrix::CreateScale(0.5);
	// Batモデルの正面方向が移動方向の計算と逆向きなので、
	// 180度回転を足して、進行方向を向いて移動するようにする。
	Math::Matrix rotMat = Math::Matrix::CreateRotationY(m_angle + DirectX::XM_PI);
	Math::Matrix transMat = Math::Matrix::CreateTranslation(m_pos);
	m_mWorld = scaleMat * rotMat * transMat;
}

void Bat::DrawLit()
{
	if (!m_spModel) { return; }

	const Math::Color drawColor = (m_hitFlashFrame > 0) ? kRedColor : kWhiteColor;
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld, drawColor);

	if (m_hitFlashFrame > 0)
	{
		--m_hitFlashFrame;
	}
}

void Bat::OnHit()
{
	// 引数なしで呼ばれた場合は、仮の基本ダメージを使う。
	// 実際の魔法攻撃では、杖ごとのダメージを反映できる OnHit(float) を使う。
	OnHit(10.0f);
}

void Bat::OnHit(float damage)
{
	// すでに死亡処理が入っている場合は、二重に経験値が入らないように何もしない。
	if (m_isExpired) { return; }

	// 0以下のダメージは無効にする。
	// 回復や特殊処理を入れる場合は、別の関数として分ける。
	if (damage <= 0.0f) { return; }

	// 魔法が当たったので、受け取ったダメージ量だけHPを減らす。
	m_hp -= damage;
	m_hitFlashFrame = 1;

	// HPが0以下になったら、BaseScene::PreUpdateで削除されるようにする。
	if (m_hp <= 0.0f)
	{
		m_isExpired = true;

		// コウモリを倒した報酬として、プレイヤーのStatusへ経験値を渡す。
		// Statusが存在しない場合は、経験値加算だけ行わずに敵の消滅はそのまま行う。
		std::shared_ptr<Status> spStatus = m_wpStatus.lock();
		if (spStatus)
		{
			spStatus->AddExp(m_exp);
		}
	}
}









