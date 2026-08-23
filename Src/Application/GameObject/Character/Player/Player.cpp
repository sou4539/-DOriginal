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

// Playerの初期化処理。
void Player::Init()
{
	// プレイヤーモデルを読み込む。
	if (!m_spModel)
	{
		m_spModel = std::make_shared<KdModelWork>();
		m_spModel->SetModelData("Asset/Models/Objects/Character/Witch/Witch.gltf");
	}

	// プレイヤーの初期位置。
	m_respawnPos = DefaultRespawnPos;
	m_pos = m_respawnPos;

	// KdGameObject側のワールド行列にも初期位置を反映する。
	SetPos(m_pos);
}

// Playerの毎フレーム更新。
void Player::Update()
{
	// CharaBase側の基本更新を呼ぶ。
	CharaBase::Update();

	UpdateInvincible();
	if (m_isControlEnable)
	{
		UpdateMove();
	}
	UpdateWorldMatrix();
}

// 無敵時間の更新処理。
void Player::UpdateInvincible()
{
	if (m_damageCoolTime <= 0.0f) { return; }

	// 無敵時間を1フレームずつ減らす。
	m_damageCoolTime -= 1.0f;
}

// プレイヤーの移動処理。
void Player::UpdateMove()
{
	// WASD入力から移動したい方向を作る。
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
		// 斜め移動時に速度が速くならないよう、方向ベクトルを正規化する。
		moveDir.Normalize();

		// カメラがない場合は、入力方向をそのまま移動方向として使う。
		m_dir = moveDir;

		std::shared_ptr<CameraBase> spCamera = m_wpCamera.lock();
		if (spCamera)
		{
			// カメラのY回転だけを使い、入力方向をカメラ基準の方向へ変換する。
			m_dir = Math::Vector3::TransformNormal(moveDir, spCamera->GetRotationYMatrix());
			m_dir.Normalize();
		}

		// 実際にプレイヤー座標を移動させる。
		m_pos += m_dir * PlayerMoveSpeed;

		// 移動方向からY軸回転角度を作る。
		m_angle = atan2(m_dir.x, m_dir.z);
	}
}

// プレイヤーのワールド行列を作る処理。
void Player::UpdateWorldMatrix()
{
	// プレイヤーのワールド行列を作る。
	Math::Matrix m_scale = Math::Matrix::CreateScale(1);
	Math::Matrix m_rot = Math::Matrix::CreateRotationY(m_angle);
	Math::Matrix m_trans = Math::Matrix::CreateTranslation(m_pos);
	m_mWorld = m_scale * m_rot * m_trans;
}

// Update後の補正・判定処理。
void Player::PostUpdate()
{
	// CharaBase側で地面や壁との当たり判定を行う。
	CharaBase::PostUpdate();

	// CharaBaseの当たり判定で補正された座標を、Player側のm_posにも反映する。
	m_pos = GetPos();

	// 現在位置が村の安全地帯内かどうかを更新する。
	UpdateSafeAreaFlag();

	// 移動と地形補正が終わった後の正しい座標で、
	UpdateDamageCollision();

	// ダメージ判定の結果HPが0になった場合は、タイトルへ戻らず村の中で復活する。
	RespawnIfDead();
}

// 敵との接触ダメージ判定。
void Player::UpdateDamageCollision()
{
	// 安全地帯内では敵との接触ダメージを受けない。
	if (m_isInSafeArea) { return; }

	// 無敵時間中はダメージを受けない。
	if (m_damageCoolTime > 0.0f) { return; }

	// HPはStatusが持っているため、まずStatusを取得する。
	std::shared_ptr<Status> spStatus = m_status.lock();
	if (!spStatus) { return; }

	// プレイヤーの体を球として扱う。
	DirectX::BoundingSphere playerSphere;
	playerSphere.Center = GetPos() + Math::Vector3(0.0f, PlayerDamageSphereHeight, 0.0f);
	playerSphere.Radius = PlayerDamageRadius;

	// TypeDamageだけを見るSphereInfoを作る。
	KdCollider::SphereInfo sphereInfo(KdCollider::TypeDamage, playerSphere);

	// 現在のシーンに存在する全オブジェクトを調べる。
	const std::list<std::shared_ptr<KdGameObject>>& objList = SceneManager::Instance().GetObjList();
	for (const std::shared_ptr<KdGameObject>& spObj : objList)
	{
		// 空のポインタは無視する。
		if (!spObj) { continue; }

		// 自分自身とは判定しない。
		if (spObj.get() == this) { continue; }

		// 対象オブジェクトがTypeDamageのコライダーを持っていて、
		std::list<KdCollider::CollisionResult> retList;
		if (spObj->Intersects(sphereInfo, &retList))
		{
			// ダメージ判定に触れたので、プレイヤーHPを5減らす。
			spStatus->DamagePlayer(BatContactDamage);

			// 次のダメージまで約1秒待つ。
			m_damageCoolTime = DamageCoolTimeFrame;

			// 1体でも当たっていれば、今回のダメージ処理は終わり。
			break;
		}
	}
}

// HPが0になった時の復活処理。
void Player::RespawnIfDead()
{
	// HPはStatus側で管理しているため、まずStatusを取得する。
	std::shared_ptr<Status> spStatus = m_status.lock();
	if (!spStatus) { return; }

	// HPがまだ残っているなら復活処理は不要。
	if (!spStatus->IsPlayerDead()) { return; }

	// プレイヤーを村の復活地点へ戻す。
	m_pos = m_respawnPos;
	SetPos(m_respawnPos);

	// 復活直後のワールド行列もすぐ正しい位置にしておく。
	UpdateWorldMatrix();

	// HPを最大まで回復する。
	spStatus->ResetPlayerHp();

	// 復活直後にコウモリへ触れていても、すぐ再ダメージを受けないようにする。
	m_damageCoolTime = RespawnInvincibleFrame;
}

void Player::UpdateSafeAreaFlag()
{
	// 半径が0以下なら、安全地帯が未設定なのでfalseにする。
	if (m_safeAreaRadius <= 0.0f)
	{
		m_isInSafeArea = false;
		return;
	}

	// XZ平面上で、プレイヤーが村の安全地帯スフィア内にいるか確認する。
	Math::Vector3 toPlayer = GetPos() - m_safeAreaCenter;
	toPlayer.y = 0.0f;

	float distanceSqr = toPlayer.LengthSquared();
	m_isInSafeArea = distanceSqr <= m_safeAreaRadius * m_safeAreaRadius;
}



