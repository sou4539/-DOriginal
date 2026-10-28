#include "DamageText.h"

namespace
{
	// Number.pngは0～9が横一列に並んでいる。
	constexpr int NumberSrcW = 50;
	constexpr int NumberSrcH = 100;
	constexpr int NumberDrawW = 18;
	constexpr int NumberDrawH = 36;

}

void DamageText::Init()
{
	m_numberTex = std::make_shared<KdTexture>();
	m_numberTex->Load("Asset/Textures/UI/LevelUp/Number.png");

	m_color = kWhiteColor;
	m_type = MagicType::None;

	m_value = 0.0f;
	m_pos = Math::Vector3::Zero;
	m_vec = Math::Vector3::Zero;
	m_ViewCount = 0;
}

void DamageText::Update()
{
	++m_ViewCount;

	// マイフレームテキストを上げる
	m_pos.y += 0.02f;

	if (m_ViewCount > 40)
	{
		m_color.w = (60 - m_ViewCount) / 20.0f;
	}

	if (m_ViewCount >= 60)
	{
		m_color.w = 0.0f;
		m_isExpired = true;
	}
}

void DamageText::DrawSprite()
{
	// フライテキスト
	// 描画に使われているカメラ情報を取得
	const auto& camera =
		KdShaderManager::Instance().GetCameraCB();

	// ワールド座標から、画面に投影するための座標へ変換
	Math::Vector4 clipPos = Math::Vector4::Transform(
		Math::Vector4(m_pos.x, m_pos.y, m_pos.z, 1.0f),
		camera.mView * camera.mProj);

	// カメラの後ろは描画しない
	if (clipPos.w <= 0.0f) { return; }

	// 現在の描画領域の大きさを取得
	Math::Viewport viewport;
	KdDirect3D::Instance().CopyViewportInfo(viewport);

	// 画面中心を原点としたスプライト座標へ変換
	const int x = static_cast<int>(
		clipPos.x / clipPos.w * viewport.width * 0.5f);

	const int y = static_cast<int>(
		clipPos.y / clipPos.w * viewport.height * 0.5f);

	if (!m_numberTex) { return; }
	if (!m_numberTex->GetSRView()) { return; }

	if (m_value < 0) { m_value = 0; }

	std::string text = std::to_string(static_cast<int>(m_value));
	for (int i = 0; i < static_cast<int>(text.size()); ++i)
	{
		const int digit = text[i] - '0';
		if (digit < 0 || digit > 9) { continue; }

		Math::Rectangle srcRect =
		{
			digit * NumberSrcW,
			0,
			NumberSrcW,
			NumberSrcH
		};

		KdShaderManager::Instance().m_spriteShader.DrawTex
		(
			m_numberTex.get(),
			x + (i * NumberDrawW),
			y,
			NumberDrawW,
			NumberDrawH,
			&srcRect,
			&m_color,
			{ 0.0f, 0.0f }
		);
	}


}

void DamageText::SetType(MagicType _type)
{
	m_type = _type;
	// ダメージの種類に応じて色を変える
	switch (m_type)
	{
		case MagicType::Fire:
		{
			//m_color = {1.0f, 0.0f, 0.0f, 1.0f};
			m_numberTex->Load("Asset/Textures/UI/DamageText/Number_Fire.png");
			break;
		}
		case MagicType::Ice:
		{
			//m_color = {0.0f, 0.0f, 1.0f, 1.0f};
			m_numberTex->Load("Asset/Textures/UI/DamageText/Number_Ice.png");
			break;
		}
		case MagicType::Volt:
		{
			//m_color = {1.0f, 1.0f, 0.0f, 1.0f};
			m_numberTex->Load("Asset/Textures/UI/DamageText/Number_Volt.png");
			break;
		}
		case MagicType::None:
		default:
		{
			//m_color = kWhiteColor;
			m_numberTex->Load("Asset/Textures/UI/LevelUp/Number.png");
			break;
		}
	}
}
