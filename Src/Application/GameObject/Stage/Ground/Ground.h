#pragma once

#include "../StageBase.h"

class Ground : public StageBase
{
public:
	Ground() { Init(); }
	~Ground() override {}

	void Update() override;
	void DrawLit() override;

	// 色画像とノーマル画像をまとめて設定する。失敗時は元の設定を維持する。
	bool SetGroundTextures(const std::string& colorFile, const std::string& normalFile = "");

	// 地面表示の基準になる対象を設定する。
	void SetTarget(const std::shared_ptr<KdGameObject>& target)
	{
		m_wpTarget = target;
	}

private:
	void Init() override;

	// ステージごとの色画像。未指定ならモデルに付属する画像を使う。
	std::shared_ptr<KdTexture> m_groundTexture;
	// 地面の色画像と、凹凸を表現するノーマルマップ。
	std::shared_ptr<KdTexture> m_groundNormalTexture;

	// 地面の表示サイズ。
	float m_groundScale = 100.0f;

	// 地面画像の繰り返し回数。
	Math::Vector2 m_textureTiling = { 10.0f, 10.0f };

	// 地面1枚ごとの配置間隔。
	float m_tileLength = 200.0f;

	// 3×3地面の中心座標。
	Math::Vector3 m_basePos = Math::Vector3::Zero;

	// 地面表示の基準対象。
	std::weak_ptr<KdGameObject> m_wpTarget;
};
