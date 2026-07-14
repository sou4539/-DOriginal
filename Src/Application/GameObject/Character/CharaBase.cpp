#include "CharaBase.h"

// 初期化
void CharaBase::Init()
{}

// 更新
void CharaBase::Update()
{
	
}

void CharaBase::PostUpdate()
{
	// Updateで移動した後の座標を使って衝突判定を行う。
	// 地面や壁にめり込んでいた場合は、ここで正しい位置へ補正する。
	UpdateCollision();
}

// 描画
void CharaBase::DrawLit()
{
	//if (m_spPoly)
	//{
	//	KdShaderManager::Instance().
	//		m_StandardShader.DrawPolygon(*m_spPoly, m_mWorld);
	//}

	if (m_spModel)
	{
		KdShaderManager::Instance().
			m_StandardShader.DrawModel(*m_spModel, m_mWorld);
	}
}

void CharaBase::UpdateCollision()
{
	// ============================================================
	// 地面との当たり判定
	//
	// キャラクターの少し上から真下へレイ（線）を飛ばし、
	// 地面に当たった位置へキャラクターを移動させる。
	// これにより、落下後に地面へ着地したり、低い段差へ乗ったりできる。
	// ============================================================
	// ----- ----- ----- ----- -----

	// ① レイ判定に必要な情報を作る。
	KdCollider::RayInfo rayInfo;

	// 現在のキャラクター位置をレイの開始位置にする。
	rayInfo.m_pos = GetPos();

	// キャラクター位置より少し高い場所からレイを飛ばす。
	// この高さまでの段差なら、地面として検出して上へ乗ることができる。
	static float enableStepHigh = 0.2f;
	rayInfo.m_pos.y += enableStepHigh;

	// レイを真下へ飛ばす。
	rayInfo.m_dir = Math::Vector3::Down;

	// 落下量と段差許容高さを合わせた長さだけレイを伸ばす。
	// m_Gravityが大きいほど、1フレームで下へ移動する量も大きくなる想定。
	rayInfo.m_range = m_Gravity + enableStepHigh;

	// Ground属性を持つコライダーだけを地面判定の対象にする。
	rayInfo.m_type = KdCollider::TypeGround;

	// 今フレームで乗っているオブジェクトを調べ直すため、一度解除する。
	m_wpRiddenObject.reset();

	// ② 登録されている当たり判定対象を1つずつ調べる。
	// weak_ptrで保持しているため、対象が削除済みならlockに失敗して無視する。
	for (std::weak_ptr<KdGameObject> wpGameObj : m_wpHitObjectList)
	{
		std::shared_ptr<KdGameObject> spGameObj = wpGameObj.lock();
		if (spGameObj)
		{
			// 1つのオブジェクト内で複数箇所に当たる可能性があるため、
			// 判定結果をリストですべて受け取る。
			std::list<KdCollider::CollisionResult> retRayList;
			spGameObj->Intersects(rayInfo, &retRayList);

			// ③ レイの判定結果から、座標補正に使う地面を選ぶ。
			// overlapDistanceが最も大きい結果を採用することで、
			// レイ開始位置に最も近い、上側の地面へ補正する。
			float maxOverLap = 0;
			Math::Vector3 hitPos = {};
			bool hit = false;
			for (auto& ret : retRayList)
			{
				// 現在までで最も補正量が大きい結果を保存する。
				if (maxOverLap < ret.m_overlapDistance)
				{
					maxOverLap = ret.m_overlapDistance;
					hitPos = ret.m_hitPos;
					hit = true;
				}
			}
			if (hit)
			{
				// 地面との交点へキャラクターを移動して、めり込みを解消する。
				SetPos(hitPos);

				// 着地したので落下量を0へ戻す。
				m_Gravity = 0;

				// 動く床など、乗ることができるオブジェクトだった場合の処理。
				if (spGameObj->IsRideable())
				{
					// キャラクターのワールド行列を乗り物のローカル空間へ変換する。
					// この相対位置を記録しておくと、乗り物が動いた時にも
					// キャラクターを同じ位置関係のまま追従させられる。
					Math::Matrix _mInvertRideObject;
					spGameObj->GetMatrix().Invert(_mInvertRideObject);

					m_mLocalFromRideObject = m_mWorld * _mInvertRideObject;
					m_wpRiddenObject = spGameObj;
				}
			}
		}
	}

	// ============================================================
	// 壁・障害物との当たり判定
	//
	// キャラクターの体を球として扱い、登録されたステージのCOLに重なっていたら、
	// 重なっている方向と距離を使って外側へ押し戻す。
	//
	// 現在は村のコライダーをTypeGroundに統一しているため、
	// スフィア判定でもTypeGroundを見るようにしている。
	// ============================================================
	// ----- ----- ----- ----- -----

	// ① 球判定に必要な情報を作る。
	DirectX::BoundingSphere sphere;

	// GetPosは足元の座標なので、球の中心を1.0だけ上へずらす。
	// 柵など少し高い位置にある障害物にも当たりやすくする。
	sphere.Center = GetPos() + Math::Vector3(0, 1.0f, 0);
	sphere.Radius = 0.5f;

	// Ground属性を持つコライダーを壁・障害物としても判定する。
	// ※将来的に地面と壁を分けたくなったら、ここをTypeBumpに戻し、
	//   ステージ側にもTypeBumpのコライダーを登録する。
	KdCollider::SphereInfo spherInfo(KdCollider::TypeGround, sphere);

	// ② 登録されている当たり判定対象を1つずつ調べる。
	for (std::weak_ptr<KdGameObject> wpGameObj : m_wpHitObjectList)
	{
		std::shared_ptr<KdGameObject> spGameObj = wpGameObj.lock();
		if (spGameObj)
		{
			// 球と対象オブジェクトのすべての衝突結果を受け取る。
			std::list<KdCollider::CollisionResult> retBumpList;
			spGameObj->Intersects(spherInfo, &retBumpList);

			// ③ 複数当たった場合は、一番深くめり込んでいる結果を使う。
			float maxOverLap = 0.0f;
			Math::Vector3 hitDir = Math::Vector3::Zero;
			bool hit = false;

			for (auto& ret : retBumpList)
			{
				if (maxOverLap < ret.m_overlapDistance)
				{
					maxOverLap = ret.m_overlapDistance;
					hitDir = ret.m_hitDir;
					hit = true;
				}
			}

			if (hit)
			{
				// m_hitDirは押し戻す方向、m_overlapDistanceは重なった距離。
				// 方向 × 距離を現在位置へ足して、めり込みを解消する。
				Math::Vector3 newPos = GetPos() + (hitDir * maxOverLap);
				SetPos(newPos);
			}
		}
	}
}

// 解放
void CharaBase::Release()
{
	m_spPoly = nullptr;
	m_spModel = nullptr;
}
