#include "CharaBase.h"

#include "../Stage/StageBase.h"

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
	UpdateCollision();
}

// 描画
void CharaBase::DrawLit()
{
	//if (m_spPoly)


	if (m_spModel)
	{
		KdShaderManager::Instance().
			m_StandardShader.DrawModel(*m_spModel, m_mWorld);
	}
}

void CharaBase::UpdateCollision()
{
	// 当たり判定対象がないキャラは、レイやスフィアを作る必要がない。
	if (m_wpHitObjects.empty()) { return; }

	// ============================================================

	// ① 球判定に必要な情報を作る。
	DirectX::BoundingSphere sphere;

	// GetPosは足元の座標なので、球の中心を1.0だけ上へずらす。
	sphere.Center = GetPos() + Math::Vector3(0, 1.0f, 0);
	sphere.Radius = 0.5f;

	// Ground属性を持つコライダーを壁・障害物としても判定する。
	KdCollider::SphereInfo sphereInfo(KdCollider::TypeGround, sphere);

	// ② 登録されている当たり判定対象を1つずつ調べる。
	for (const std::weak_ptr<KdGameObject>& wpObject : m_wpHitObjects)
	{
		std::shared_ptr<KdGameObject> spObject = wpObject.lock();
		if (spObject)
		{
			// StageBaseを継承しているオブジェクトは、
			std::shared_ptr<StageBase> spStage = std::dynamic_pointer_cast<StageBase>(spObject);
			if (spStage && !spStage->EnableSphereCollision())
			{
				continue;
			}

			// 球と対象オブジェクトのすべての衝突結果を受け取る。
			std::list<KdCollider::CollisionResult> hits;
			spObject->Intersects(sphereInfo, &hits);

			// ③ 複数当たった場合は、一番深くめり込んでいる結果を使う。
			float maxDepth = 0.0f;
			Math::Vector3 pushDir = Math::Vector3::Zero;
			bool hasHit = false;

			for (auto& hit : hits)
			{
				if (maxDepth < hit.m_overlapDistance)
				{
					maxDepth = hit.m_overlapDistance;
					pushDir = hit.m_hitDir;
					hasHit = true;
				}
			}

			if (hasHit)
			{
				// m_hitDirは押し戻す方向、m_overlapDistanceは重なった距離。
				Math::Vector3 newPos = GetPos() + (pushDir * maxDepth);
				newPos.y = GetPos().y;	// Y座標は変えない。
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
