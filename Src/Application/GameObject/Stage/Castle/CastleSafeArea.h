#pragma once
#include "../../Character/Player/Player.h"
#include <cmath>
#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iomanip>

// 城の安全地帯と、その編集画面。開始位置・復活位置とは独立して管理する。
// コライダーによる押し戻しは不要なので、中心と大きさだけで判定する。
class CastleSafeArea : public KdGameObject
{
public:
	explicit CastleSafeArea(const std::shared_ptr<Player>& player) : m_player(player)
	{
		m_pDebugWire = std::make_unique<KdDebugWireFrame>();
		m_preview.Set2DObject(false);
		LoadSettings();
		ApplySettings();
	}

	void Update() override
	{
		// Zで表示するワイヤー枠も、判定と同じ中心・サイズを使う。
		if (m_showBounds)
		{
			// AddDebugBoxは半分の大きさを受け取るため、全サイズを半分にする。
			m_pDebugWire->AddDebugBox(Math::Matrix::CreateTranslation(m_center), m_size * 0.5f,
				Math::Vector3::Zero, false, kGreenColor);
		}
	}



	void DrawEffect() override
	{
		if (!m_showBounds || !KdDebugWireFrame::IsEnable()) { return; }
		const float y = std::clamp(0.0f, m_center.y - m_size.y * 0.5f,
			m_center.y + m_size.y * 0.5f) + 0.04f;
		const auto rotation = Math::Matrix::CreateRotationX(DirectX::XM_PIDIV2);
		auto& shader = KdShaderManager::Instance().m_StandardShader;
		auto draw = [&](float width, float depth, float x, float z, float alpha)
		{
			const auto world = Math::Matrix::CreateScale(width, depth, 1.0f) * rotation
				* Math::Matrix::CreateTranslation(x, y + (alpha > 0.5f ? 0.01f : 0.0f), z);
			shader.DrawPolygon(m_preview, world, Math::Color(0.1f, 1.0f, 0.2f, alpha));
		};
		draw(m_size.x, m_size.z, m_center.x, m_center.z, 0.12f);
		for (float side : {-1.0f, 1.0f})
		{
			draw(m_size.x, 0.08f, m_center.x, m_center.z + side * m_size.z * 0.5f, 0.9f);
			draw(0.08f, m_size.z, m_center.x + side * m_size.x * 0.5f, m_center.z, 0.9f);
		}
		for (const auto& corner : m_corners)
		{
			const auto world = Math::Matrix::CreateScale(0.6f, 0.6f, 1.0f) * rotation
				* Math::Matrix::CreateTranslation(corner.x, y + 0.02f, corner.y);
			shader.DrawPolygon(m_preview, world, Math::Color(1.0f, 0.5f, 0.05f, 1.0f));
		}
	}

private:
	static bool IsValid(const Math::Vector3& center, const Math::Vector3& size)
	{
		return std::isfinite(center.x) && std::isfinite(center.y) && std::isfinite(center.z) &&
			std::isfinite(size.x) && std::isfinite(size.y) && std::isfinite(size.z) &&
			size.x > 0.0f && size.y > 0.0f && size.z > 0.0f;
	}

	void ApplySettings()
	{
		if (auto player = m_player.lock()) { player->SetSafeAreaBox(m_center, m_size); }
		SyncCorners();
	}

	void SyncCorners()
	{
		const auto minimum = m_center - m_size * 0.5f;
		const auto maximum = m_center + m_size * 0.5f;
		m_corners = { Math::Vector2(minimum.x, minimum.z), Math::Vector2(maximum.x, minimum.z),
			Math::Vector2(maximum.x, maximum.z), Math::Vector2(minimum.x, maximum.z) };
		m_cornerPending = false;
	}

	void ApplyCorners()
	{
		Math::Vector2 minimum = m_corners[0];
		Math::Vector2 maximum = m_corners[0];
		for (const auto& corner : m_corners)
		{
			if (!std::isfinite(corner.x) || !std::isfinite(corner.y))
			{
				m_message = U8("角の座標に有効な数値を入力してください。");
				return;
			}
			minimum.x = std::min(minimum.x, corner.x);
			minimum.y = std::min(minimum.y, corner.y);
			maximum.x = std::max(maximum.x, corner.x);
			maximum.y = std::max(maximum.y, corner.y);
		}
		const Math::Vector2 size = maximum - minimum;
		const Math::Vector2 center = minimum + size * 0.5f;
		const Math::Vector3 newCenter(center.x, m_center.y, center.y);
		const Math::Vector3 newSize(size.x, m_size.y, size.y);
		if (!IsValid(newCenter, newSize) || size.x < 0.1f || size.y < 0.1f)
		{
			m_message = U8("幅・奥行きが小さすぎます。離れた角を指定してください。");
			return;
		}
		m_center = newCenter;
		m_size = newSize;
		ApplySettings();
		m_message = U8("長方形を反映しました。次回も使うには保存してください。");
	}

	void LoadSettings()
	{
		std::ifstream file("Save/CastleSafeArea.txt");
		if (!file) { m_message = U8("保存データがありません。現在の設定を使用します。"); return; }
		Math::Vector3 center, size;
		if (!(file >> center.x >> center.y >> center.z >> size.x >> size.y >> size.z) ||
			!IsValid(center, size))
		{
			m_message = U8("保存データが不正なため、現在の設定を維持します。");
			return;
		}
		m_center = center;
		m_size = size;
		m_message = U8("保存した設定を読み込みました。");
	}

	void SaveSettings()
	{
		// フォルダがない場合も作成し、書き込み失敗を編集画面に表示する。
		std::error_code error;
		std::filesystem::create_directories("Save", error);
		if (error) { m_message = U8("保存フォルダを作成できません。"); return; }
		std::ofstream file("Save/CastleSafeArea.txt");
		file << std::setprecision(9) << m_center.x << ' ' << m_center.y << ' ' << m_center.z << '\n'
			<< m_size.x << ' ' << m_size.y << ' ' << m_size.z << '\n';
		file.flush();
		m_message = file ? U8("保存しました。次回もこの設定を使用します。") : U8("保存に失敗しました。");
	}

	std::weak_ptr<Player> m_player;
	KdSquarePolygon m_preview;
	std::array<Math::Vector2, 4> m_corners{};
	bool m_cornerPending = false;
	Math::Vector3 m_center = { -30.0f, 0.0f, 0.0f };
	// 部屋に合わせてエディターで調整する仮の全幅・全高・全奥行き。
	Math::Vector3 m_size = { 20.0f, 6.0f, 20.0f };
	bool m_showBounds = true;
	std::string m_message;
};
