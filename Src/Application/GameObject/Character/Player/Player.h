#pragma once
#include "../CharaBase.h"

class Status;
class CameraBase;

class Player : public CharaBase
{
public:
	// Playerを作成した時に、自動でInit()を呼んで初期化する。
	// GameSceneでは std::make_shared<Player>() するだけで、モデルと初期座標が準備される。
	Player() { Init(); }

	// Player破棄時の処理。
	// CharaBase側のデストラクタでモデル解放を行うため、ここでは追加処理を持たせていない。
	~Player() override {}

	// 毎フレームの通常更新。
	// 入力、移動、向き、ワールド行列の更新を行う。
	void Update() override;

	// Update後に呼ばれる更新。
	// 地面・壁の当たり判定、敵とのダメージ判定、死亡時の復活判定を行う。
	void PostUpdate() override;

	// Playerがダメージを受けた時に操作するStatusを登録する。
	// GameSceneでStatusを作成した後、player->SetStatus(status) の形で呼ぶ。
	void SetStatus(const std::shared_ptr<Status>& status)
	{
		m_status = status;
	}

	// カメラ基準で移動するために、現在使っているカメラを登録する。
	// 登録しておくと、WASD移動がカメラの向きに合わせた方向になる。
	void SetCamera(const std::shared_ptr<CameraBase>& camera)
	{
		m_wpCamera = camera;
	}

	// HPが0になった時に戻る座標を外から設定する。
	// 村の位置を変えた時に、Player.cppの中を書き換えなくて済むようにしている。
	void SetRespawnPos(const Math::Vector3& respawnPos)
	{
		m_respawnPos = respawnPos;
	}

	// 村の安全地帯スフィアを設定する。
	// この範囲内にいる間は、敵がプレイヤーを追わないようにする。
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
	// タイトル画面など、見た目としてPlayerを表示したいがWASDでは動かしたくない時に使う。
	void SetControlEnable(bool enable) { m_isControlEnable = enable; }

	// 外部から表示位置を設定する。
	// KdGameObject側の座標だけでなく、Player内部で使っているm_posも一緒に更新する。
	void SetPos(const Math::Vector3& pos) override
	{
		m_pos = pos;
		KdGameObject::SetPos(pos);
	}

	// タイトル画面などで、プレイヤーの向きだけを指定したい時に使う。
	void SetAngle(float angle) { m_angle = angle; }

private:
	// Playerの初期化処理。
	// モデル読み込み、初期位置、復活地点の初期値を設定する。
	void Init() override;

	// 無敵時間を更新する。
	// ダメージ処理と分けておくことで、Updateの流れを読みやすくする。
	void UpdateInvincible();

	// 入力とカメラ向きから移動方向を作り、プレイヤーを移動させる。
	void UpdateMove();

	// m_posとm_angleから、描画用のワールド行列を作る。
	void UpdateWorldMatrix();

	// プレイヤーの体用スフィアが、TypeDamageの当たり判定に触れているか確認する。
	// コウモリはTypeDamageを持っているので、コウモリ接触ダメージの確認に使う。
	void UpdateDamageCollision();

	// HPが0になっているか確認し、0なら村の復活地点へ戻す。
	// このゲームではリザルトやタイトルへ戻らず、村の中で復活する。
	void RespawnIfDead();

	// プレイヤーが村の安全地帯内にいるか確認する。
	void UpdateSafeAreaFlag();

	// プレイヤーHPはStatusが管理している。
	// Playerはダメージが発生した時だけ、StatusのDamagePlayerを呼ぶ。
	std::weak_ptr<Status> m_status;

	// WASD入力をカメラ基準の移動方向へ変換するために使う。
	// weak_ptrにして、PlayerがCameraを勝手に生存させ続けないようにしている。
	std::weak_ptr<CameraBase> m_wpCamera;

	// プレイヤーのY軸回転角度。
	// 移動方向から角度を作り、モデルを進行方向へ向けるために使う。
	float m_angle = 0.0f;

	// ダメージを受けた後の無敵時間。
	// この値が0より大きい間は、コウモリに触れていても追加ダメージを受けない。
	// これがないと、接触中に毎フレームHPが減ってしまう。
	float m_damageCoolTime = 0.0f;

	// HPが0になった時に戻る村の中の座標。
	// 今はプレイヤー初期位置と同じ場所を復活地点として使う。
	Math::Vector3 m_respawnPos = Math::Vector3::Zero;

	// 村の安全地帯にいるかどうか。
	// Batなどの敵はこのフラグを見て、追跡するか初期位置へ戻るかを決める。
	bool m_isInSafeArea = false;

	// 村を覆う安全地帯スフィア。
	Math::Vector3 m_safeAreaCenter = Math::Vector3::Zero;
	float m_safeAreaRadius = 0.0f;

	// trueならWASD入力で移動する。
	// falseならモデル表示だけ行い、タイトル画面の飾りとして使えるようにする。
	bool m_isControlEnable = true;
};





