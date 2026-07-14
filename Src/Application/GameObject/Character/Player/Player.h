#pragma once
#include "../CharaBase.h"

class Status;
class CameraBase;

class Player : public CharaBase
{
public:
	Player() { Init(); }
	~Player()				override	{}

	
	void Update()			override;
	void PostUpdate()		override;
	//void DrawLit()			override;

	void SetStatus(const std::shared_ptr<Status>& status) 
	{ 
		m_status = status;
	}

	// カメラの向きを基準に移動方向を決めるため、GameSceneからカメラを受け取る。
	void SetCamera(const std::shared_ptr<CameraBase>& camera)
	{
		m_wpCamera = camera;
	}
private:
	std::weak_ptr<Status> m_status;

	// weak_ptrで持つことで、カメラが削除された後に参照し続けないようにする。
	std::weak_ptr<CameraBase> m_wpCamera;

	// プレイヤーの向き。移動方向からY軸回転を作るために使う。
	float m_angle = 0.0f;

	void Init()				override;
};
