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

	void DrawDebugGui() override
	{
		ImGui::SetNextWindowSize(ImVec2(440, 0), ImGuiCond_FirstUseEver);
		if (ImGui::Begin(U8("城の安全地帯##CastleSafeArea")))
		{
			ImGui::TextWrapped(U8("1. 四つ角を登録 → 2. 長方形を反映 → 3. 保存"));
			ImGui::TextWrapped(U8("緑の床表示が範囲の目安です。範囲内では接触ダメージを受けません。"));
			ImGui::Separator();
			ImGui::TextWrapped(U8("各角へ移動し、F1でマウス操作に切り替えて登録してください。角の順番は自由です。"));
			ImGui::TextWrapped(U8("四点のX・Zの端から、辺がX・Z方向に沿う長方形を作ります。斜めの長方形にはなりません。"));
			for (int i = 0; i < 4; ++i)
			{
				ImGui::PushID(i);
				ImGui::Text(U8("角 %d"), i + 1);
				ImGui::SameLine();
				float corner[2] = { m_corners[i].x, m_corners[i].y };
				ImGui::SetNextItemWidth(180);
				if (ImGui::DragFloat2("X / Z", corner, 0.1f))
				{
					m_corners[i] = Math::Vector2(corner[0], corner[1]);
					m_cornerPending = true;
				}
				if (auto player = m_player.lock())
				{
					ImGui::SameLine();
					if (ImGui::Button(U8("現在位置を登録")))
					{
						const auto pos = player->GetPos();
						m_corners[i] = Math::Vector2(pos.x, pos.z);
						m_cornerPending = true;
					}
				}
				ImGui::PopID();
			}
			ImGui::TextWrapped(U8("オレンジの印が指定した角、緑が現在の安全地帯です。高さは下の全高で調整できます。"));
			if (ImGui::Button(U8("四つ角から長方形を反映"))) { ApplyCorners(); }
			if (m_cornerPending)
			{
				ImGui::TextWrapped(U8("角の変更は未反映です。反映ボタンを押してから保存してください。"));
			}
			ImGui::Separator();
			ImGui::TextUnformatted(U8("中心・高さ・広さの微調整"));
			ImGui::TextWrapped(U8("数値をドラッグして変更。Ctrl+クリックで直接入力できます。"));
			float center[3] = { m_center.x, m_center.y, m_center.z };
			float size[3] = { m_size.x, m_size.y, m_size.z };
			bool changed = ImGui::DragFloat3(U8("中心 X / Y / Z"), center, 0.1f);
			ImGui::TextWrapped(U8("X: 左右、Y: 高さ、Z: 奥行き。中心は箱の真ん中です。"));
			changed |= ImGui::DragFloat3(U8("全幅 / 全高 / 全奥行き"), size, 0.1f, 0.1f, 500.0f);
			ImGui::TextWrapped(U8("広さ20なら中心から両側へ10ずつ。半径ではありません。"));
			if (changed)
			{
				Math::Vector3 newCenter(center[0], center[1], center[2]);
				Math::Vector3 newSize(size[0], size[1], size[2]);
				if (IsValid(newCenter, newSize))
				{
					m_center = newCenter;
					m_size = newSize;
					ApplySettings();
					m_message = U8("変更は反映済みです。次回も使うには保存してください。");
				}
			}
			ImGui::Checkbox(U8("床に範囲を表示する"), &m_showBounds);
			ImGui::TextWrapped(U8("床表示はZ不要。高さを含む箱の枠を見る場合はZを押してください。"));
			if (auto player = m_player.lock())
			{
				if (ImGui::Button(U8("プレイヤーの現在位置を中心にする")))
				{
					m_center = player->GetPos();
					ApplySettings();
					m_message = U8("中心を変更しました。次回も使うには保存してください。");
				}
				const auto pos = player->GetPos();
				ImGui::Text(U8("現在位置 X: %.2f Y: %.2f Z: %.2f"), pos.x, pos.y, pos.z);
				ImGui::Text("%s", player->IsInSafeArea() ? U8("判定：安全地帯の中") : U8("判定：安全地帯の外"));
			}
			const auto minimum = m_center - m_size * 0.5f;
			const auto maximum = m_center + m_size * 0.5f;
			ImGui::Text(U8("X範囲 %.1f ～ %.1f"), minimum.x, maximum.x);
			ImGui::Text(U8("Y範囲 %.1f ～ %.1f（足元で判定）"), minimum.y, maximum.y);
			ImGui::Text(U8("Z範囲 %.1f ～ %.1f"), minimum.z, maximum.z);
			ImGui::Separator();
			if (ImGui::Button(U8("保存する")))
			{
				if (m_cornerPending) { m_message = U8("角の変更を先に反映してください。"); }
				else { SaveSettings(); }
			}
			ImGui::SameLine();
			if (ImGui::Button(U8("保存した設定に戻す"))) { LoadSettings(); ApplySettings(); }
			ImGui::TextWrapped("%s", m_message.c_str());
		}
		ImGui::End();
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
