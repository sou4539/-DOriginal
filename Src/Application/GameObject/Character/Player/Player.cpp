#include "Player.h"

#include "../../Camera/CameraBase.h"
#include "../Status/Status.h"
#include "../Enemy/EnemyBase.h"
#include "../../../Scene/SceneManager.h"

#include<cmath>

namespace
{
	constexpr float PlayerMoveSpeed = 0.15f;
	constexpr float PlayerDamageRadius = 1.2f;
	constexpr float PlayerDamageSphereHeight = 1.5f;
	constexpr float BatContactDamage = 5.0f;
	constexpr float HitInvincibleFrames = 60.0f;
	constexpr float RespawnInvincibleFrames = 120.0f;
	constexpr int PlayerHitFlashFrames = 12;

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

void Player::DrawLit()
{
	if (!m_spModel) { return; }

	auto& shader = KdShaderManager::Instance().m_StandardShader;
	// 接触ダメージを受けた直後だけ赤い輪郭を表示する。
	// DrawModel終了時に設定が戻るため、影や他のキャラクターには伝わらない。
	shader.SetLimLightEnable(m_hitFlashFrames > 0);
	if (m_hitFlashFrames > 0)
	{
		shader.SetLimLight({ 3.0f, 0.1f, 0.1f }, 2.0f);
	}
	shader.DrawModel(*m_spModel, FlyMat());
}

void Player::GenerateDepthMapFromLight()
{
	if (!m_spModel) { return; }
	auto& shader = KdShaderManager::Instance().m_StandardShader;

	// 通常描画と同じモデル行列で描くことで、見た目と同じ形の影を作る。
	shader.DrawModel(*m_spModel, FlyMat());
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
	if (m_hitFlashFrames > 0) { --m_hitFlashFrames; }
	if (m_canMove)
	{
		UpdateMove();
	}
	UpdateWorldMatrix();

	m_FlyAngle += DirectX::XM_2PI / 120.0f;
	if (m_FlyAngle >= DirectX::XM_2PI)
	{
		m_FlyAngle -= DirectX::XM_2PI;
	}
}

// KdDebugGUIのNewFrameとRenderの間で呼び出す。
void Player::DrawDebugGui()
{
	// 現在位置を確認するためのデバッグウィンドウ。
	ImGui::Begin("Player Position");

	const Math::Vector3 pos = GetPos();
	ImGui::Text("X: %.3f  Y: %.3f  Z: %.3f", pos.x, pos.y, pos.z);

	if (ImGui::Button("Set Spawn Position"))
	{
		// 現在位置を復活位置として登録する。
		SetRespawnPos(pos);
	}

	ImGui::End();
}

// 無敵時間を更新する。
void Player::UpdateInvincible()
{
	if (m_invincibleFrames <= 0.0f) { return; }

	// 無敵時間を1フレームずつ減らす。
	m_invincibleFrames -= 1.0f;
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
	Math::Matrix scaleMat = Math::Matrix::CreateScale(1);
	Math::Matrix rotMat = Math::Matrix::CreateRotationY(m_angle);
	Math::Matrix transMat = Math::Matrix::CreateTranslation(m_pos);
	m_mWorld = scaleMat * rotMat * transMat;
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
	if (m_invincibleFrames > 0.0f) { return; }

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
		std::list<KdCollider::CollisionResult> hits;
		if (spObj->Intersects(sphereInfo, &hits))
		{
			// 敵に触れたのでプレイヤーHPを減らす。
			// 敵ごとの接触ダメージを使う。草原のコウモリは従来どおり5。
			auto enemy = std::dynamic_pointer_cast<EnemyBase>(spObj);
			spStatus->DamagePlayer(enemy ? enemy->GetContactDamage() : BatContactDamage);

			// 次のダメージまで少し待つ。
			m_invincibleFrames = HitInvincibleFrames;
			m_hitFlashFrames = PlayerHitFlashFrames;

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
	m_invincibleFrames = RespawnInvincibleFrames;
	m_hitFlashFrames = 0;
}

void Player::UpdateSafeAreaFlag()
{
	if (m_useSafeAreaBox)
	{
		// 足元の座標が箱の内側なら安全。四隅も含めて判定する。
		const Math::Vector3 offset = GetPos() - m_safeAreaCenter;
		const Math::Vector3 halfSize = m_safeAreaBoxSize * 0.5f;
		m_isInSafeArea = std::abs(offset.x) <= halfSize.x &&
			std::abs(offset.y) <= halfSize.y && std::abs(offset.z) <= halfSize.z;
		return;
	}
	// 半径が0以下なら安全地帯は未設定として扱う。
	if (m_safeAreaRadius <= 0.0f)
	{
		m_isInSafeArea = false;
		return;
	}

	// XZ平面上で、プレイヤーが安全地帯の球範囲内にいるか確認する。
	Math::Vector3 toPlayer = GetPos() - m_safeAreaCenter;
	toPlayer.y = 0.0f;

	float distSq = toPlayer.LengthSquared();
	m_isInSafeArea = distSq <= m_safeAreaRadius * m_safeAreaRadius;
}

Math::Matrix Player::FlyMat() const
{
	// プレイヤーの浮遊処理(見た目だけ)
	const float flyHeight = 0.4f + std::sin(m_FlyAngle) * 0.1f;

	Math::Matrix drawMat = m_mWorld;
	Math::Vector3 pos = drawMat.Translation();
	pos.y += flyHeight;
	drawMat.Translation(pos);

	return drawMat;
}



