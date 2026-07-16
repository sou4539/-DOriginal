#include "TPSCamera.h"

void TPSCamera::Init()
{
	// 親クラスの初期化呼び出し
	CameraBase::Init();

	// プレイヤーから見たカメラの相対位置。
	// yを高くすると見下ろしやすくなり、zをマイナスにするとプレイヤーの後ろへ下がる。
	m_mLocalPos = Math::Matrix::CreateTranslation(0, 10.0f, -15.0f);

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);
}

void TPSCamera::PostUpdate()
{
	// ターゲットの座標(有効な場合利用する)
	Math::Vector3								_targetPos = Math::Vector3::Zero;
	const std::shared_ptr<const KdGameObject>	_spTarget = m_wpTarget.lock();
	if (_spTarget)
	{
		_targetPos = _spTarget->GetPos();
		// 足元ではなく、プレイヤーの体の中心あたりを見る。
		// これにより画面中央にプレイヤーが入りやすくなる。
		_targetPos.y += 1.0f;
	}

	// カメラの回転
	UpdateRotateByMouse();

	// マウス左右の回転だけを使って、プレイヤーの周囲を回り込む。
	m_mRotation = GetRotationYMatrix();

	// プレイヤーから見たカメラまでの距離。
	// この値をプレイヤー座標に足して、カメラの座標として使う。
	Math::Vector3 _cameraDistance = Math::Vector3(0, 15.0f, -15.0f);

	// カメラまでの距離を、マウス左右の回転に合わせて回す。
	// これで「プレイヤーを中心に、一定距離を保って回り込む」動きになる。
	Math::Vector3 _cameraOffset = Math::Vector3::TransformNormal(_cameraDistance, m_mRotation);
	Math::Vector3 _cameraPos = _targetPos + _cameraOffset;

	// カメラからプレイヤーへ向かう方向。
	// カメラの向きはこの方向にするので、必ずプレイヤーの位置を見る。
	Math::Vector3 _toTarget = _targetPos - _cameraPos;
	_toTarget.Normalize();

	// カメラのワールド行列を「座標」と「向き」から作る。
	// CreateWorldは内部で第2引数のforwardを反転してZ軸に使うため、
	// カメラがプレイヤーを見るように、ここでは逆向きのベクトルを渡す。
	m_mWorld = Math::Matrix::CreateWorld(_cameraPos, -_toTarget, Math::Vector3::Up);

	// ↓めり込み防止の為の座標補正計算↓
	// ①当たり判定(レイ判定)用の情報作成
	KdCollider::RayInfo rayInfo;
	// レイの発射位置を設定
	rayInfo.m_pos = GetPos();

	// レイの発射方向を設定
	rayInfo.m_dir = Math::Vector3::Down;
	// レイの長さを設定
	rayInfo.m_range = 1000.f;
	if (_spTarget)
	{
		rayInfo.m_dir = _targetPos - GetPos();
		rayInfo.m_range = rayInfo.m_dir.Length();
		rayInfo.m_dir.Normalize();
	}

	// 当たり判定をしたいタイプを設定
	rayInfo.m_type = KdCollider::TypeGround;

	// ②HIT判定対象オブジェクトに総当たり
	for (std::weak_ptr<KdGameObject> wpGameObj : m_wpHitObjectList)
	{
		std::shared_ptr<KdGameObject> spGameObj = wpGameObj.lock();
		if (spGameObj)
		{
			std::list<KdCollider::CollisionResult> retRayList;
			spGameObj->Intersects(rayInfo, &retRayList);

			// ③ 結果を使って座標を補完する
			// レイに当たったリストから一番近いオブジェクトを検出
			float maxOverLap = 0;
			Math::Vector3 hitPos = {};
			bool hit = false;
			for (auto& ret : retRayList)
			{
				// レイを遮断しオーバーした長さが
				// 一番長いものを探す
				if (maxOverLap < ret.m_overlapDistance)
				{
					maxOverLap = ret.m_overlapDistance;
					hitPos = ret.m_hitPos;
					hit = true;
				}
			}
			if (hit)
			{
				// 何かしらの障害物に当たっている
				Math::Vector3 _hitPos = hitPos;
				_hitPos += rayInfo.m_dir * 0.4f;
				SetPos(_hitPos);
			}
		}
	}
}
