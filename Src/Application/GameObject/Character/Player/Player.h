#pragma once
#include "../CharaBase.h"

class Status;
class CameraBase;

class Player : public CharaBase
{
public:
	// Playerを作成した時に、自動でInit()を呼んで初期化する。
	Player() { Init(); }

	// Player破棄時の処理。
	~Player() override {}

	// 毎フレームの通常更新。
	void Update() override;

	// Update後に呼ばれる更新。
	void PostUpdate() override;

	// Playerがダメージを受けた時に操作するStatusを登録する。
	void SetStatus(const std::shared_ptr<Status>& status)
	{
		m_status = status;
	}

	// カメラ基準で移動するために、現在使っているカメラを登録する。
	void SetCamera(const std::shared_ptr<CameraBase>& camera)
	{
		m_wpCamera = camera;
	}

	// HPが0になった時に戻る座標を外から設定する。
	void SetRespawnPos(const Math::Vector3& respawnPos)
	{
		m_respawnPos = respawnPos;
	}

	// 村の安全地帯スフィアを設定する。
	void SetSafeArea(const Math::Vector3& center, float radius)
	{
		m_safeAreaCenter = center;
		m_safeAreaRadius = radius;

		Math::Vector3 toPlayer = GetPos() - m_safeAreaCenter;
		toPlayer.y = 0.0f;
		m_isInSafeArea = toPlayer.LengthSquared() <= m_safeAreaRadius * m_safeAreaRadius;
	}

	// プレイヤーが安全地帯にいるかを返す。
	bool IsInSafeArea() const { return m_isInSafeArea; }

	// プレイヤー操作の有効/無効を切り替える。
	void SetControlEnable(bool enable) { m_isControlEnable = enable; }

	// 外部から表示位置を設定する。
	void SetPos(const Math::Vector3& pos) override
	{
		m_pos = pos;
		KdGameObject::SetPos(pos);
	}

	// タイトル画面などで、プレイヤーの向きだけを指定したい時に使う。
	void SetAngle(float angle) { m_angle = angle; }

private:
	// Playerの初期化処理。
	void Init() override;

	// 無敵時間を更新する。
	void UpdateInvincible();

	// 入力とカメラ向きから移動方向を作り、プレイヤーを移動させる。
	void UpdateMove();

	// m_posとm_angleから、描画用のワールド行列を作る。
	void UpdateWorldMatrix();

	// プレイヤーの体用スフィアが、TypeDamageの当たり判定に触れているか確認する。
	void UpdateDamageCollision();

	// HPが0になっているか確認し、0なら村の復活地点へ戻す。
	void RespawnIfDead();

	// プレイヤーが村の安全地帯内にいるか確認する。
	void UpdateSafeAreaFlag();

	// プレイヤーHPはStatusが管理している。
	std::weak_ptr<Status> m_status;

	// WASD入力をカメラ基準の移動方向へ変換するために使う。
	std::weak_ptr<CameraBase> m_wpCamera;

	// プレイヤーのY軸回転角度。
	float m_angle = 0.0f;

	// ダメージを受けた後の無敵時間。
	float m_damageCoolTime = 0.0f;

	// HPが0になった時に戻る村の中の座標。
	Math::Vector3 m_respawnPos = Math::Vector3::Zero;

	// 村の安全地帯にいるかどうか。
	bool m_isInSafeArea = false;

	// 村を覆う安全地帯スフィア。
	Math::Vector3 m_safeAreaCenter = Math::Vector3::Zero;
	float m_safeAreaRadius = 0.0f;

	// trueならWASD入力で移動する。
	bool m_isControlEnable = true;
};





