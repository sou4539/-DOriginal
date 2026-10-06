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
	if (!m_isMouseLocked) { return; }

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);
	m_skipMouseMove = true;
}

void CameraBase::SetMouseLocked(bool isLocked)
{
	if (m_isMouseLocked == isLocked) { return; }

	m_isMouseLocked = isLocked;
	m_skipMouseMove = false;

	if (m_isMouseLocked)
	{
		ResetMouseMove();
	}
}

void CameraBase::UpdateRotateByMouse()
{
	if (!m_isMouseLocked) { return; }

	// マウスでカメラを横回転させる。
	POINT mousePos;
	GetCursorPos(&mousePos);

	if (m_skipMouseMove)
	{
		SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);
		m_skipMouseMove = false;
		return;
	}

	POINT mouseDelta{};
	mouseDelta.x = mousePos.x - m_FixMousePos.x;
	mouseDelta.y = mousePos.y - m_FixMousePos.y;

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);

	// 上下方向は使わず、左右の移動量だけを反映する。
	m_anglesDeg.y += mouseDelta.x * 0.15f;
}
