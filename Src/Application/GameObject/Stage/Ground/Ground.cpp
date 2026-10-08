#include "Ground.h"

bool Ground::SetGroundTextures(const std::string& colorFile, const std::string& normalFile)
{
	// 空文字ならモデルに付属する画像へ戻す。
	if (colorFile.empty())
	{
		if (!normalFile.empty()) { return false; }
		m_groundTexture.reset();
		m_groundNormalTexture.reset();
		return true;
	}

	// 2枚とも読み込めてから反映する。片方が失敗しても現在の床を維持する。
	auto colorTexture = std::make_shared<KdTexture>();
	if (!colorTexture->Load(colorFile)) { return false; }
	std::shared_ptr<KdTexture> normalTexture;
	if (!normalFile.empty())
	{
		normalTexture = std::make_shared<KdTexture>();
		if (!normalTexture->Load(normalFile)) { return false; }
	}
	m_groundTexture = colorTexture;
	m_groundNormalTexture = normalTexture;
	return true;
}

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
			// 共有マテリアルを変更せず、このタイルの画像だけ差し替える。
			if (m_groundTexture)
			{
				KdShaderManager::Instance().m_StandardShader.SetBaseColorTextureOverride(m_groundTexture);
				KdShaderManager::Instance().m_StandardShader.SetNormalTextureOverride(m_groundNormalTexture);
			}
			KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, drawMat);
		}
	}

	KdShaderManager::Instance().UndoSamplerState();
}

