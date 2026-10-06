#pragma once

class CameraBase : public KdGameObject
{
public:
	CameraBase()						{}
	virtual ~CameraBase()	override	{}

	void Init()				override;
	void PreDraw()			override;

	void SetTarget(const std::shared_ptr<KdGameObject>& target);

	// カメラの横回転角度を外から指定する。
	void SetYawDeg(float yawDeg) { m_anglesDeg.y = yawDeg; }

	// UI操作後に、次のカメラ更新でマウス移動量を使わないようにする。
	void ResetMouseMove();
	void SetMouseLocked(bool isLocked);

	// UIなどがカメラ基準の方向を計算する時に使う。
	float GetYawDeg() const { return m_anglesDeg.y; }

	// 「絶対変更しません！見るだけ！」な書き方
	const std::shared_ptr<KdCamera>& GetCamera() const
	{
		return m_spCamera;
	}

	// 「中身弄るかもね」な書き方
	std::shared_ptr<KdCamera> WorkCamera() const
	{
		return m_spCamera;
	}

	const Math::Matrix GetRotationMatrix()const
	{
		return Math::Matrix::CreateFromYawPitchRoll(
		       DirectX::XMConvertToRadians(m_anglesDeg.y),
		       DirectX::XMConvertToRadians(m_anglesDeg.x),
		       DirectX::XMConvertToRadians(m_anglesDeg.z));
	}

	const Math::Matrix GetRotationYMatrix() const
	{
		return Math::Matrix::CreateRotationY(
			   DirectX::XMConvertToRadians(m_anglesDeg.y));
	}

	void RegistHitObject(const std::shared_ptr<KdGameObject>& object)
	{
		m_wpHitObjects.push_back(object);
	}

protected:
	// カメラ回転用角度
	Math::Vector3								m_anglesDeg = Math::Vector3::Zero;

	void UpdateRotateByMouse();

	std::shared_ptr<KdCamera>					m_spCamera = nullptr;
	std::weak_ptr<KdGameObject>					m_wpTarget;
	std::vector<std::weak_ptr<KdGameObject>>	m_wpHitObjects{};

	Math::Matrix								m_mLocalPos = Math::Matrix::Identity;
	Math::Matrix								m_mRotation = Math::Matrix::Identity;

	// カメラ回転用マウス座標の差分
	POINT										m_FixMousePos{};
	bool										m_skipMouseMove = false;
	bool										m_isMouseLocked = true;
};
