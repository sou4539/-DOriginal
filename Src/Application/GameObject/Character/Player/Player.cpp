#include "Player.h"
#include "../../Camera/CameraBase.h"

void Player::Init()
{
	// プレイヤーの見た目モデルを読み込む。
	// すでにモデルがある場合は再読み込みしない。
	if (!m_spModel)
	{
		m_spModel = std::make_shared<KdModelWork>();
		m_spModel->SetModelData("Asset/Models/Objects/Witch/Witch.gltf");
	}

	// 現在の初期位置。
	// 村やグラウンドの配置確認をしやすいように、少し左側から開始している。
	m_pos = { -30,0,0 };

	// KdGameObject側の座標も初期位置に合わせる。
	SetPos(m_pos);
}

void Player::Update()
{
	CharaBase::Update();

	// WASD入力から「移動したい方向」を作る。
	// この時点ではカメラを考慮しない、ワールド基準の入力方向。
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
		// 斜め移動時に速度が速くならないように正規化する。
		moveDir.Normalize();

		// 実際に使う移動方向。
		// カメラがなければ、入力方向をそのまま使う。
		m_dir = moveDir;

		std::shared_ptr<CameraBase> spCamera = m_wpCamera.lock();
		if (spCamera)
		{
			// カメラのY回転だけを使って、入力方向をカメラ基準の方向へ回転させる。
			// 上下の角度は使わないので、地面に沿って移動できる。
			m_dir = Math::Vector3::TransformNormal(moveDir, spCamera->GetRotationYMatrix());
			m_dir.Normalize();
		}

		// カメラ基準に変換した方向へ移動する。
		const float moveSpeed = 0.1f;
		m_pos += m_dir * moveSpeed;

		// 移動方向からプレイヤーの向きを作る。
		// xとzからY軸回転角度を求めることで、進行方向を向く。
		m_angle = atan2(m_dir.x, m_dir.z);
	}

	// 拡大・回転・移動の順にワールド行列を作る。
	// 回転にはm_angleを使うため、移動方向へ体が向く。
	Math::Matrix m_scale = Math::Matrix::CreateScale(1);
	Math::Matrix m_rot = Math::Matrix::CreateRotationY(m_angle);
	Math::Matrix m_trans = Math::Matrix::CreateTranslation(m_pos);
	m_mWorld = m_scale * m_rot * m_trans;
}

void Player::PostUpdate()
{
	// CharaBase側で地面・壁との当たり判定を行う。
	CharaBase::PostUpdate();

	// 当たり判定でSetPosされた場合に、Player側のm_posも同じ座標へ合わせる。
	// これをしないと、次のUpdateで補正前のm_posに戻ってしまう。
	m_pos = GetPos();
}
