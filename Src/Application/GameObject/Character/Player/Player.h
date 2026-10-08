#pragma once
#include "../CharaBase.h"

class Status;
class CameraBase;

class Player : public CharaBase
{
public:
	// 生成時にInitを呼び、プレイヤーを初期化する。
	Player() { Init(); }

	// プレイヤー破棄時の処理。
	~Player() override {}

	// 毎フレームの通常更新。
	void Update() override;

	// 被弾直後だけ輪郭を光らせて、モデルを描画する。
	void DrawLit() override;

	// ImGuiのフレーム開始後に呼ぶ。Updateから直接呼ばない。
	void DrawDebugGui() override;

	// 影を落とすため、ライト視点の深度マップへプレイヤーモデルを描画する。
	void GenerateDepthMapFromLight() override;

	// 地面に接地感を出すため、簡易的な黒い影を描画する。
	void DrawEffect() override;

	// Update後に呼ばれる補正・判定処理。
	void PostUpdate() override;

	// プレイヤーがダメージを受けた時に操作するStatusを登録する。
	void SetStatus(const std::shared_ptr<Status>& status)
	{
		m_status = status;
	}

	// カメラ基準で移動するため、現在使っているカメラを登録する。
	void SetCamera(const std::shared_ptr<CameraBase>& camera)
	{
		m_wpCamera = camera;
	}

	// HPが0になった時に戻る復活座標を設定する。
	void SetRespawnPos(const Math::Vector3& respawnPos)
	{
		m_respawnPos = respawnPos;
	}

	// 村の安全地帯スフィアを設定する。
	void SetSafeArea(const Math::Vector3& center, float radius)
	{
		m_useSafeAreaBox = false;
		m_safeAreaCenter = center;
		m_safeAreaRadius = radius;

		Math::Vector3 toPlayer = GetPos() - m_safeAreaCenter;
		toPlayer.y = 0.0f;
		m_isInSafeArea = toPlayer.LengthSquared() <= m_safeAreaRadius * m_safeAreaRadius;
	}

	// プレイヤーが安全地帯にいるか返す。
	bool IsInSafeArea() const { return m_isInSafeArea; }

	// 城の安全地帯。sizeは半径ではなく、箱全体の幅・高さ・奥行き。
	void SetSafeAreaBox(const Math::Vector3& center, const Math::Vector3& size)
	{
		m_useSafeAreaBox = true;
		m_safeAreaCenter = center;
		m_safeAreaBoxSize = size;
		UpdateSafeAreaFlag();
	}

	// プレイヤー操作の有効/無効を切り替える。
	void SetControlEnable(bool enable) { m_canMove = enable; }

	// 外部から表示位置を設定する。
	void SetPos(const Math::Vector3& pos) override
	{
		m_pos = pos;
		KdGameObject::SetPos(pos);
	}

	// タイトル画面などで、プレイヤーの向きを指定したい時に使う。
	void SetAngle(float angle) { m_angle = angle; }

private:
	// プレイヤーの初期設定。
	void Init() override;

	// 無敵時間を更新する。
	void UpdateInvincible();

	// 入力とカメラ方向から移動方向を作り、プレイヤーを移動させる。
	void UpdateMove();

	// m_posとm_angleから描画用のワールド行列を作る。
	void UpdateWorldMatrix();

	// プレイヤーの体用スフィアが、TypeDamage判定に触れているか確認する。
	void UpdateDamageCollision();

	// HPが0なら村の復活地点へ戻す。
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
	float m_invincibleFrames = 0.0f;

	// 見た目の被弾時間。無敵時間とは分け、復活時には光らせない。
	int m_hitFlashFrames = 0;

	// HPが0になった時に戻る村の中の座標。
	Math::Vector3 m_respawnPos = Math::Vector3::Zero;

	// 村の安全地帯にいるかどうか。
	bool m_isInSafeArea = false;

	// 村を覆う安全地帯スフィア。
	Math::Vector3 m_safeAreaCenter = Math::Vector3::Zero;
	float m_safeAreaRadius = 0.0f;
	bool m_useSafeAreaBox = false;
	Math::Vector3 m_safeAreaBoxSize = Math::Vector3::Zero;

	// trueならWASD入力で移動できる。
	bool m_canMove = true;

	// プレイヤーの浮遊処理(見た目だけ)
	float m_FlyAngle = 0.0f;
	Math::Matrix FlyMat() const;
};





