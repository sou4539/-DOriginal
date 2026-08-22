#include "Ground.h"

void Ground::Init()
{
	// 地面モデルと当たり判定を準備する。
	m_spModel = std::make_shared<KdModelWork>();
	m_spModel->SetModelData("Asset/Models/Objects/Stage/Ground/Ground.gltf");

	m_mWorld = Math::Matrix::CreateScale(m_groundScale);

	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape
	(
		"Ground",
		m_spModel,
		KdCollider::TypeGround
	);
}

void Ground::Update()
{
	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();
	if (!spTarget) { return; }

	// プレイヤー位置を地面1枚分の区切りに丸める。
	Math::Vector3 targetPos = spTarget->GetPos();
	m_basePos.x = std::floor((targetPos.x / m_tileLength) + 0.5f) * m_tileLength;
	m_basePos.y = 0.0f;
	m_basePos.z = std::floor((targetPos.z / m_tileLength) + 0.5f) * m_tileLength;

	// 当たり判定は中心の地面に合わせる。
	m_mWorld = Math::Matrix::CreateScale(m_groundScale) *
			   Math::Matrix::CreateTranslation(m_basePos);
}

void Ground::DrawLit()
{
	if (!m_spModel) { return; }

	// 地面だけ補間なし・繰り返しありで描画する。
	KdShaderManager::Instance().ChangeSamplerState(KdSamplerState::Point_Wrap);

	// プレイヤー周辺を埋めるため、3×3枚の地面を描画する。
	for (int z = -1; z <= 1; ++z)
	{
		for (int x = -1; x <= 1; ++x)
		{
			Math::Vector3 drawPos = m_basePos;
			drawPos.x += m_tileLength * x;
			drawPos.z += m_tileLength * z;

			Math::Matrix drawMat = Math::Matrix::CreateScale(m_groundScale) *
								   Math::Matrix::CreateTranslation(drawPos);

			// DrawModel後にUV設定が戻るため、描画ごとに設定する。
			KdShaderManager::Instance().m_StandardShader.SetUVTiling(m_textureTiling);
			KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, drawMat);
		}
	}

	KdShaderManager::Instance().UndoSamplerState();
}

