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
// 使い方：
//   Player生成時にコンストラクタから自動で呼ばれる。
//   基本的に外から直接呼び直さない。
// 処理内容：
//   モデルを読み込み、初期位置と復活地点を設定する。
void Player::Init()
{
	// プレイヤーモデルを読み込む。
	// すでに読み込み済みの場合は、同じモデルを二重に読み込まない。
	if (!m_spModel)
	{
		m_spModel = std::make_shared<KdModelWork>();
		m_spModel->SetModelData("Asset/Models/Objects/Character/Witch/Witch.gltf");
	}

	// プレイヤーの初期位置。
	// m_posは移動計算で使うPlayer側の座標。
	m_respawnPos = DefaultRespawnPos;
	m_pos = m_respawnPos;

	// KdGameObject側のワールド行列にも初期位置を反映する。
	SetPos(m_pos);
}

// Playerの毎フレーム更新。
// 使い方：
//   シーンのUpdate処理から毎フレーム自動で呼ばれる。
// 処理内容：
//   無敵時間、移動入力、描画用ワールド行列を更新する。
void Player::Update()
{
	// CharaBase側の基本更新を呼ぶ。
	// 今は処理が少なくても、基底クラス側の処理を残すために呼んでおく。
	CharaBase::Update();

	UpdateInvincible();
	UpdateMove();
	UpdateWorldMatrix();
}

// 無敵時間の更新処理。
// 使い方：
//   Update()の中から毎フレーム呼ぶ。
// 処理内容：
//   ダメージ後や復活後に設定した無敵時間を1フレームずつ減らす。
void Player::UpdateInvincible()
{
	if (m_damageCoolTime <= 0.0f) { return; }

	// 無敵時間を1フレームずつ減らす。
	// 60FPS想定なら、60フレームで約1秒になる。
	m_damageCoolTime -= 1.0f;
}

// プレイヤーの移動処理。
// 使い方：
//   Update()の中から毎フレーム呼ぶ。
// 処理内容：
//   WASD入力を確認し、カメラの向きに合わせて移動方向を作る。
//   斜め移動が速くならないようにNormalizeで正規化する。
void Player::UpdateMove()
{
	// WASD入力から移動したい方向を作る。
	// この時点ではまだカメラの向きは考慮していない。
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
			// 上下回転を使わないため、プレイヤーは地面に沿って移動できる。
			m_dir = Math::Vector3::TransformNormal(moveDir, spCamera->GetRotationYMatrix());
			m_dir.Normalize();
		}

		// 実際にプレイヤー座標を移動させる。
		m_pos += m_dir * PlayerMoveSpeed;

		// 移動方向からY軸回転角度を作る。
		// これにより、プレイヤーモデルが進行方向を向く。
		m_angle = atan2(m_dir.x, m_dir.z);
	}
}

// プレイヤーのワールド行列を作る処理。
// 使い方：
//   m_posやm_angleを変更した後に呼ぶ。
// 処理内容：
//   拡大、回転、移動を合成して、モデルの表示位置と向きを決める。
void Player::UpdateWorldMatrix()
{
	// プレイヤーのワールド行列を作る。
	// 拡大、回転、移動の順に合成して、モデルの表示位置と向きを決める。
	Math::Matrix m_scale = Math::Matrix::CreateScale(1);
	Math::Matrix m_rot = Math::Matrix::CreateRotationY(m_angle);
	Math::Matrix m_trans = Math::Matrix::CreateTranslation(m_pos);
	m_mWorld = m_scale * m_rot * m_trans;
}

// Update後の補正・判定処理。
// 使い方：
//   シーンのPostUpdate処理から毎フレーム自動で呼ばれる。
// 処理内容：
//   地形との当たり判定で位置を補正し、その後に敵接触ダメージと復活判定を行う。
void Player::PostUpdate()
{
	// CharaBase側で地面や壁との当たり判定を行う。
	// めり込みがあった場合は、SetPosで座標が補正される。
	CharaBase::PostUpdate();

	// CharaBaseの当たり判定で補正された座標を、Player側のm_posにも反映する。
	// これをしないと、次のUpdateで補正前の座標に戻ってしまう。
	m_pos = GetPos();

	// 移動と地形補正が終わった後の正しい座標で、
	// コウモリなどとのダメージ判定を確認する。
	UpdateDamageCollision();

	// ダメージ判定の結果HPが0になった場合は、タイトルへ戻らず村の中で復活する。
	RespawnIfDead();
}

// 敵との接触ダメージ判定。
// 使い方：
//   PostUpdate()で、地形との位置補正が終わった後に呼ぶ。
// 処理内容：
//   プレイヤーの体をスフィアとして扱い、TypeDamageのコライダーと重なっているか調べる。
//   重なっていた場合はStatusへダメージ処理を依頼し、連続ヒット防止用の無敵時間を設定する。
void Player::UpdateDamageCollision()
{
	// 無敵時間中はダメージを受けない。
	// 連続ヒットでHPが一瞬で0になるのを防ぐため。
	if (m_damageCoolTime > 0.0f) { return; }

	// HPはStatusが持っているため、まずStatusを取得する。
	// 取得できない場合はHPを減らせないので、ここで終了する。
	std::shared_ptr<Status> spStatus = m_status.lock();
	if (!spStatus) { return; }

	// プレイヤーの体を球として扱う。
	// コウモリは少し高い位置を飛ぶため、球の中心を足元より上へずらしている。
	DirectX::BoundingSphere playerSphere;
	playerSphere.Center = GetPos() + Math::Vector3(0.0f, PlayerDamageSphereHeight, 0.0f);
	playerSphere.Radius = PlayerDamageRadius;

	// TypeDamageだけを見るSphereInfoを作る。
	// これにより、地面や壁ではなく、敵のダメージ判定だけを対象にできる。
	KdCollider::SphereInfo sphereInfo(KdCollider::TypeDamage, playerSphere);

	// 現在のシーンに存在する全オブジェクトを調べる。
	// BatGroupで追加したBatも、このリストの中に入っている。
	const std::list<std::shared_ptr<KdGameObject>>& objList = SceneManager::Instance().GetObjList();
	for (const std::shared_ptr<KdGameObject>& spObj : objList)
	{
		// 空のポインタは無視する。
		if (!spObj) { continue; }

		// 自分自身とは判定しない。
		if (spObj.get() == this) { continue; }

		// 対象オブジェクトがTypeDamageのコライダーを持っていて、
		// playerSphereと重なっていればtrueになる。
		std::list<KdCollider::CollisionResult> retList;
		if (spObj->Intersects(sphereInfo, &retList))
		{
			// ダメージ判定に触れたので、プレイヤーHPを5減らす。
			spStatus->DamagePlayer(BatContactDamage);

			// 次のダメージまで約1秒待つ。
			// 60FPS想定なので60フレームにしている。
			m_damageCoolTime = DamageCoolTimeFrame;

			// 1体でも当たっていれば、今回のダメージ処理は終わり。
			break;
		}
	}
}

// HPが0になった時の復活処理。
// 使い方：
//   ダメージ判定後に呼び、HPが0なら村の復活地点へ戻す。
// 処理内容：
//   プレイヤー座標をm_respawnPosへ戻し、HPを最大まで回復し、復活直後の無敵時間を付ける。
void Player::RespawnIfDead()
{
	// HPはStatus側で管理しているため、まずStatusを取得する。
	// 取得できない場合は復活判定ができないので何もしない。
	std::shared_ptr<Status> spStatus = m_status.lock();
	if (!spStatus) { return; }

	// HPがまだ残っているなら復活処理は不要。
	if (!spStatus->IsPlayerDead()) { return; }

	// プレイヤーを村の復活地点へ戻す。
	// m_posとKdGameObject側の座標を両方更新しないと、
	// 次のUpdateで古いm_posからワールド行列が作られて位置が戻ってしまう。
	m_pos = m_respawnPos;
	SetPos(m_respawnPos);

	// 復活直後のワールド行列もすぐ正しい位置にしておく。
	// これで復活したフレームから表示位置が村の中になる。
	UpdateWorldMatrix();

	// HPを最大まで回復する。
	spStatus->ResetPlayerHp();

	// 復活直後にコウモリへ触れていても、すぐ再ダメージを受けないようにする。
	// 60FPS想定で約2秒の無敵時間。
	m_damageCoolTime = RespawnInvincibleFrame;
}
