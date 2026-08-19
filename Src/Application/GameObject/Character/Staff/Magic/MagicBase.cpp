#include "MagicBase.h"

#include "../../../../Scene/SceneManager.h"
#include "../../Bat/Bat.h"

void MagicBase::Init()
{
	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

	m_pos = {};
	m_dir = {};
	m_magicType = MagicType::None;
	m_damage = 0.0f;
	m_speed = 0.0f;
	m_lifeTime = 0.0f;
	m_radius = 0.0f;

	m_spPoly = std::make_shared<KdSquarePolygon>();
}

void MagicBase::Update()
{
	m_pos += m_dir * m_speed;
	m_lifeTime -= 1.0f;

	if (m_lifeTime <= 0.0f)
	{
		m_isExpired = true;
	}

	if (m_pDebugWire)
	{
		//m_pDebugWire->AddDebugSphere(m_pos, m_radius);
	}

	//移動行列
	Math::Matrix m_trans = Math::Matrix::CreateTranslation(m_pos);

	//回転行列
	Math::Matrix m_rotx = Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(90.0f));
	Math::Matrix m_roty = Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(90.0f));
	Math::Matrix m_rotY = Math::Matrix::CreateRotationY(atan2f(m_dir.x, m_dir.z));


	m_mWorld = m_rotx * m_roty * m_rotY * m_trans;
}

void MagicBase::PostUpdate()
{
	// すでに寿命切れなどで消える予定の魔法は、当たり判定を行わない。
	if (m_isExpired)
	{
		return;
	}

	// 魔法の当たり判定用スフィアを作成する。
	// TypeDamageを見ることで、敵が持っているダメージ判定に当たったかを確認する。
	DirectX::BoundingSphere magicSphere;
	magicSphere.Center = GetPos();
	magicSphere.Radius = m_radius;

	KdCollider::SphereInfo sphereInfo(KdCollider::TypeDamage, magicSphere);

	// シーン内のオブジェクトを調べ、Batに当たったらダメージを与える。
	for (const std::shared_ptr<KdGameObject>& spObj : SceneManager::Instance().GetObjList())
	{
		if (!spObj)
		{
			continue;
		}

		// 今は敵がBatだけなので、Batに変換できたものだけを攻撃対象にする。
		// 後でEnemyBaseを作ったら、ここをEnemyBase判定に変更する。
		std::shared_ptr<Bat> spBat = std::dynamic_pointer_cast<Bat>(spObj);
		if (!spBat)
		{
			continue;
		}
		if (spBat->IsExpired())
		{
			continue;
		}

		std::list<KdCollider::CollisionResult> retList;
		if (spBat->Intersects(sphereInfo, &retList))
		{
			// 敵に魔法のダメージ量を渡し、魔法自身は消す。
			// m_damageにはFire/Ice/Voltなど杖ごとの攻撃力が入っている。
			spBat->OnHit(m_damage);
			m_isExpired = true;
			break;
		}
	}
}

void MagicBase::DrawLit()
{
	if (!m_spPoly) { return; }
	KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_spPoly, m_mWorld);
}

void MagicBase::Shot(const Math::Vector3& startPos, const Math::Vector3& dir, MagicType type, float damage, float speed)
{
	m_pos = startPos;
	m_dir = dir;
	m_magicType = type;
	m_damage = damage;
	m_speed = speed;


	m_dir.Normalize();

	switch (m_magicType)
	{
	case MagicType::Fire:
		m_lifeTime = 90;
		m_radius = 0.5f;
		m_spPoly->SetMaterial("Asset/Textures/Magic/Fire/Fire.png");
		m_spPoly->SetScale(10.0f);
		break;
	case MagicType::Ice:
		m_lifeTime = 120;
		m_radius = 0.3f;
		m_spPoly->SetMaterial("Asset/Textures/Magic/Ice/Ice1.png");
		m_spPoly->SetScale(10.0f);
		break;
	case MagicType::Volt:
		m_lifeTime = 60;
		m_radius = 0.2f;
		m_spPoly->SetMaterial("Asset/Textures/Magic/Volt/Volt1.png");
		m_spPoly->SetScale(10.0f);
		break;
	default:
		break;
	}
}


