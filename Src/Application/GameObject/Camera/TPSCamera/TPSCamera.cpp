#include "TPSCamera.h"

namespace
{
	constexpr float CameraLookAheadDistance = 5.0f;
}

void TPSCamera::Init()
{
	// カメラ共通の初期化を呼ぶ。
	CameraBase::Init();

	// プレイヤーから見たカメラの相対位置。
	m_mLocalPos = Math::Matrix::CreateTranslation(0, 15.0f, -15.0f);

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);
}

void TPSCamera::PostUpdate()
{
	// ターゲットの座標を取得する。
	Math::Vector3								targetPos = Math::Vector3::Zero;
	const std::shared_ptr<const KdGameObject>	spTarget = m_wpTarget.lock();
	if (spTarget)
	{
		targetPos = spTarget->GetPos();
		// 足元ではなく、プレイヤーの体の中心あたりを見る。
		targetPos.y += 1.0f;
	}

	// マウス入力でカメラを回転させる。
	UpdateRotateByMouse();

	// マウス左右の回転を使い、プレイヤーの周囲を回り込む。
	m_mRotation = GetRotationYMatrix();

	Math::Vector3 lookAtPos = targetPos;
	if (spTarget)
	{
		// プレイヤーより少し前を見ることで、進行方向を広めに映す。
		Math::Vector3 lookAheadDir = Math::Vector3::TransformNormal(Math::Vector3(0.0f, 0.0f, 1.0f), m_mRotation);
		lookAheadDir.y = 0.0f;
		if (lookAheadDir.LengthSquared() > 0.0f)
		{
			lookAheadDir.Normalize();
			lookAtPos += lookAheadDir * CameraLookAheadDistance;
		}
	}

	// プレイヤーからカメラまでの距離。
	Math::Vector3 localOffset = Math::Vector3(0, 15.0f, -15.0f);

	// カメラまでの距離を、マウス左右の回転に合わせて回す。
	Math::Vector3 worldOffset = Math::Vector3::TransformNormal(localOffset, m_mRotation);
	Math::Vector3 cameraPos = targetPos + worldOffset;

	// カメラを前方寄りの注視点へ向ける。
	Math::Vector3 viewDir = lookAtPos - cameraPos;
	viewDir.Normalize();

	// カメラのワールド行列を座標と向きから作る。
	m_mWorld = Math::Matrix::CreateWorld(cameraPos, -viewDir, Math::Vector3::Up);

	// カメラが地形にめり込まないように補正する。
	KdCollider::RayInfo rayInfo;
	// レイの開始位置を設定する。
	rayInfo.m_pos = GetPos();

	// レイの方向を設定する。
	rayInfo.m_dir = Math::Vector3::Down;
	// レイの長さを設定する。
	rayInfo.m_range = 1000.f;
	if (spTarget)
	{
		rayInfo.m_dir = lookAtPos - GetPos();
		rayInfo.m_range = rayInfo.m_dir.Length();
		rayInfo.m_dir.Normalize();
	}

	// 地形判定を対象にする。
	rayInfo.m_type = KdCollider::TypeGround;

	// 登録された地形オブジェクトとレイ判定する。
	for (std::weak_ptr<KdGameObject> wpGameObj : m_wpHitObjects)
	{
		std::shared_ptr<KdGameObject> spGameObj = wpGameObj.lock();
		if (spGameObj)
		{
			std::list<KdCollider::CollisionResult> hits;
			spGameObj->Intersects(rayInfo, &hits);

			// 一番大きく重なった結果を使って座標を補正する。
			float maxDepth = 0;
			Math::Vector3 hitPos = {};
			bool hasHit = false;
			for (auto& hit : hits)
			{
				// レイとの重なりが一番大きい結果を採用する。
				if (maxDepth < hit.m_overlapDistance)
				{
					maxDepth = hit.m_overlapDistance;
					hitPos = hit.m_hitPos;
					hasHit = true;
				}
			}
			if (hasHit)
			{
				// 地形にめり込まない位置へカメラを戻す。
				Math::Vector3 correctedPos = hitPos;
				correctedPos += rayInfo.m_dir * 0.4f;
				SetPos(correctedPos);
			}
		}
	}
}
