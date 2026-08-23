#include "CameraBase.h"

void CameraBase::Init()
{
	if (!m_spCamera)
	{
		m_spCamera = std::make_shared<KdCamera>();
	}
	// 画面中央座標。
	m_FixMousePos.x = 640;
	m_FixMousePos.y = 360;
}

void CameraBase::PreDraw()
{
	if (!m_spCamera) { return; }

	m_spCamera->SetCameraMatrix(m_mWorld);
	m_spCamera->SetToShader();
}

void CameraBase::SetTarget(const std::shared_ptr<KdGameObject>& target)
{
	if (!target) { return; }

	m_wpTarget = target;
}

void CameraBase::ResetMouseMove()
{
	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);
	m_skipMouseMove = true;
}

void CameraBase::UpdateRotateByMouse()
{
	// マウスでカメラを横回転させる。
	POINT _nowPos;
	GetCursorPos(&_nowPos);

	if (m_skipMouseMove)
	{
		SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);
		m_skipMouseMove = false;
		return;
	}

	POINT _mouseMove{};
	_mouseMove.x = _nowPos.x - m_FixMousePos.x;
	_mouseMove.y = _nowPos.y - m_FixMousePos.y;

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);

	// 上下方向は使わず、左右の移動量だけを反映する。
	m_DegAng.y += _mouseMove.x * 0.15f;
}
