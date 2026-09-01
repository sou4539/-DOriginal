#include "Player.h"

#include "../../Camera/CameraBase.h"
#include "../Status/Status.h"
#include "../../../Scene/SceneManager.h"

namespace
{
	constexpr float PlayerMoveSpeed = 0.1f;
	constexpr float PlayerDamageRadius = 1.2f;
	constexpr float PlayerDamageSphereHeight = 1.5f;
	constexpr float BatContactDamage = 5.0f;
	constexpr float DamageCoolTimeFrame = 60.0f;
	constexpr float RespawnInvincibleFrame = 120.0f;

	const Math::Vector3 DefaultRespawnPos = { -30.0f, 0.0f, 0.0f };
}

// プレイヤーの初期設定。
void Player::Init()
{
	// プレイヤーモデルを読み込む。
	if (!m_spModel)
	{
		m_spModel = std::make_shared<KdModelWork>();
		m_spModel->SetModelData("Asset/Models/Objects/Character/Witch/Witch.gltf");
	}

	// 村の中を初期位置にする。
	m_respawnPos = DefaultRespawnPos;
	m_pos = m_respawnPos;

	// 当たり判定などで使う基底クラス側の座標にも反映する。
	SetPos(m_pos);
}

void Player::GenerateDepthMapFromLight()
{
	if (!m_spModel) { return; }

	// 通常描画と同じモデル行列で描くことで、見た目と同じ形の影を作る。
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}
void Player::DrawEffect()
{
	if (!m_spModel) { return; }

	// Fake ground shadow.
	Math::Matrix shadowMat = m_mWorld * Math::Matrix::CreateScale(1.0f, 0.0f, 1.0f);
	shadowMat.Translation({ m_pos.x, 0.03f, m_pos.z });

	const Math::Color shadowColor = { 0.0f, 0.0f, 0.0f, 0.35f };
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, shadowMat, shadowColor);
}
// 毎フレームのプレイヤー更新。
void Player::Update()
{
	// キャラクター共通の更新を行う。
	CharaBase::Update();

	UpdateInvincible();
	if (m_isControlEnable)
	{
		UpdateMove();
	}
	UpdateWorldMatrix();
}

// 無敵時間を更新する。
void Player::UpdateInvincible()
{
	if (m_damageCoolTime <= 0.0f) { return; }

	// 無敵時間を1フレームずつ減らす。
	m_damageCoolTime -= 1.0f;
}

// プレイヤーの移動処理。
void Player::UpdateMove()
{
	// WASD入力から移動方向を作る。
	Math::Vector3 moveDir = Math::Vector3::Zero;

	if (GetAsyncKeyState('W') & 0x8000)
	{
		moveDir.z += 1.0f;
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		moveDir.z -= 1.0f;
	}
	if (GetAsyncKeyState('A') & 0x8000)
	{
		moveDir.x -= 1.0f;
	}
	if (GetAsyncKeyState('D') & 0x8000)
	{
		moveDir.x += 1.0f;
	}

	if (moveDir.LengthSquared() > 0.0f)
	{
		// 斜め移動で速度が上がらないよう正規化する。
		moveDir.Normalize();

		// カメラがない場合は入力方向をそのまま使う。
		m_dir = moveDir;

		std::shared_ptr<CameraBase> spCamera = m_wpCamera.lock();
		if (spCamera)
		{
			// カメラのY回転を使い、入力方向をカメラ基準へ変換する。
			m_dir = Math::Vector3::TransformNormal(moveDir, spCamera->GetRotationYMatrix());
			m_dir.Normalize();
		}

		// 実際にプレイヤー座標を移動させる。
		m_pos += m_dir * PlayerMoveSpeed;

		// 移動方向に合わせてプレイヤーの向きを変える。
		m_angle = atan2(m_dir.x, m_dir.z);
	}
}

// プレイヤーのワールド行列を更新する。
void Player::UpdateWorldMatrix()
{
	// 座標と向きを描画用の行列へ反映する。
	Math::Matrix m_scale = Math::Matrix::CreateScale(1);
	Math::Matrix m_rot = Math::Matrix::CreateRotationY(m_angle);
	Math::Matrix m_trans = Math::Matrix::CreateTranslation(m_pos);
	m_mWorld = m_scale * m_rot * m_trans;
}

// Update後の補正と当たり判定。
void Player::PostUpdate()
{
	// キャラ共通の地形当たり判定を行う。
	CharaBase::PostUpdate();

	// 当たり判定で補正された座標をPlayer側のm_posにも反映する。
	m_pos = GetPos();

	// 現在位置が村の安全地帯内か確認する。
	UpdateSafeAreaFlag();

	// 移動と地形補正後の正しい座標でダメージ判定する。
	UpdateDamageCollision();

	// HPが0ならタイトルへ戻さず、村の中で復活させる。
	RespawnIfDead();
}

// 敵との接触ダメージ判定。
void Player::UpdateDamageCollision()
{
	// 安全地帯内では敵との接触ダメージを受けない。
	if (m_isInSafeArea) { return; }

	// 無敵時間中はダメージを受けない。
	if (m_damageCoolTime > 0.0f) { return; }

	// HPはStatusが管理しているので、まずStatusを取得する。
	std::shared_ptr<Status> spStatus = m_status.lock();
	if (!spStatus) { return; }

	// プレイヤーの体を球として扱う。
	DirectX::BoundingSphere playerSphere;
	playerSphere.Center = GetPos() + Math::Vector3(0.0f, PlayerDamageSphereHeight, 0.0f);
	playerSphere.Radius = PlayerDamageRadius;

	// TypeDamageを対象にする球判定を作る。
	KdCollider::SphereInfo sphereInfo(KdCollider::TypeDamage, playerSphere);

	// 現在のシーンにある全オブジェクトを調べる。
	const std::list<std::shared_ptr<KdGameObject>>& objList = SceneManager::Instance().GetObjList();
	for (const std::shared_ptr<KdGameObject>& spObj : objList)
	{
		// 空のポインタは無視する。
		if (!spObj) { continue; }

		// 自分自身とは判定しない。
		if (spObj.get() == this) { continue; }

		// TypeDamageのコライダーに触れているか確認する。
		std::list<KdCollider::CollisionResult> retList;
		if (spObj->Intersects(sphereInfo, &retList))
		{
			// 敵に触れたのでプレイヤーHPを減らす。
			spStatus->DamagePlayer(BatContactDamage);

			// 次のダメージまで少し待つ。
			m_damageCoolTime = DamageCoolTimeFrame;

			// 1体でも触れていたら今回の判定は終わる。
			break;
		}
	}
}

// HPが0になった時の復活処理。
void Player::RespawnIfDead()
{
	// HPはStatusが管理しているので、まずStatusを取得する。
	std::shared_ptr<Status> spStatus = m_status.lock();
	if (!spStatus) { return; }

	// HPが残っているなら復活処理は不要。
	if (!spStatus->IsPlayerDead()) { return; }

	// プレイヤーを村の復活地点へ戻す。
	m_pos = m_respawnPos;
	SetPos(m_respawnPos);

	// 復活後の座標を描画にも反映する。
	UpdateWorldMatrix();

	// HPを最大まで回復する。
	spStatus->ResetPlayerHp();

	// 復活直後に敵へ触れていても、すぐダメージを受けないようにする。
	m_damageCoolTime = RespawnInvincibleFrame;
}

void Player::UpdateSafeAreaFlag()
{
	// 半径が0以下なら安全地帯は未設定として扱う。
	if (m_safeAreaRadius <= 0.0f)
	{
		m_isInSafeArea = false;
		return;
	}

	// XZ平面上で、プレイヤーが安全地帯の球範囲内にいるか確認する。
	Math::Vector3 toPlayer = GetPos() - m_safeAreaCenter;
	toPlayer.y = 0.0f;

	float distanceSqr = toPlayer.LengthSquared();
	m_isInSafeArea = distanceSqr <= m_safeAreaRadius * m_safeAreaRadius;
}



