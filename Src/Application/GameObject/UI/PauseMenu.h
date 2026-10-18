#pragma once
#include "UI.h"
#include <functional>

// ステージが一時停止状態を管理し、このクラスは画像・配置・メニュー入力を担当する。
class PauseMenu : public UI
{
public:

	enum class Action
	{
		Resume,
		Return_To_Village,
		Return_To_Title,
	};

	PauseMenu()
	{
		m_frame = LoadTexture("Asset/Textures/UI/Menu/gold_panel.png");
		m_resume = LoadTexture("Asset/Textures/UI/Menu/ReturnToGame.png");
		m_title = LoadTexture("Asset/Textures/UI/Menu/ReturnToTitle.png");
		m_village = LoadTexture("Asset/Textures/UI/Menu/ReturnToVillage.png");
		m_cursor = LoadTexture("Asset/Textures/UI/Cursor.png");
	}
	void SetOpen(bool open)
	{
		m_open = open;
		// メニューを開く前から押されていたクリックでは、項目を選択しない。
		m_previousLeftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	}
	void SetCallback(const std::function<void(Action)>& callback) { m_callback = callback; }

	// 通常のオブジェクト更新を止めている間も、ステージから呼び出す。
	void HandleInput()
	{
		const bool leftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
		const bool clicked = leftDown && !m_previousLeftDown;
		m_previousLeftDown = leftDown;

		if (!m_open || !clicked || !m_callback) { return; }

		if (IsMouseInSprite(0, ResumeY, ButtonWidth, ButtonHeight))
		{
			PlayUIClickSound(); 
			m_callback(Action::Resume);
		}
		else if (m_recallEnabled && IsMouseInSprite(0, VillageY, ButtonWidth, ButtonHeight))
		{
			PlayUIClickSound();
			m_callback(Action::Return_To_Village);
		}
		else if (IsMouseInSprite(0, TitleY, ButtonWidth, ButtonHeight))
		{
			PlayUIClickSound();
			m_callback(Action::Return_To_Title);
		}
	}

	// HUDの後に描画し、背景を暗くする表示を画面全体に重ねる。
	void DrawMenu() const
	{
		if (!m_open) { return; }
		auto& shader = KdShaderManager::Instance().m_spriteShader;
		const Math::Color dim(0.0f, 0.0f, 0.0f, 0.7f);
		shader.DrawBox(0, 0, 640, 360, &dim, true);
		// 1枚の共通枠に、個別に選択できる2つの文字画像を配置する。
		if (m_frame) { shader.DrawTex(m_frame.get(), 0, 0, PanelWidth, PanelHeight); }
		DrawButton(m_resume.get(), ResumeY);
		if (m_recallEnabled)
		{
			DrawButton(m_village.get(), VillageY, 0.85f);
		}
		DrawButton(m_title.get(), TitleY);
		DrawCursorUI(m_cursor.get());
	}

	void SetRecallEnabled(bool enabled)
	{
		m_recallEnabled = enabled;
	}

private:
	// 枠とボタンのサイズ
	static constexpr int PanelWidth = 560;
	static constexpr int PanelHeight = 500;
	static constexpr int ButtonWidth = 500;
	static constexpr int ButtonHeight = 110;

	// ボタンの位置
	static constexpr int ResumeY = 150;
	static constexpr int VillageY = 0;
	static constexpr int TitleY = -150;

	void DrawButton(KdTexture* label, int y, float labelScale = 1.0f) const
	{
		if (!m_frame || !label) { return; }
		const bool hovered = IsMouseInSprite(0, y, ButtonWidth, ButtonHeight);
		const bool pressed = hovered && (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
		const float scale = hovered ? 1.04f : 1.0f;
		const float brightness = pressed ? 0.8f : hovered ? 1.2f : 1.0f;
		const Math::Color tint(brightness, brightness, brightness, 1.0f);
		auto& shader = KdShaderManager::Instance().m_spriteShader;
		const int labelWidth =static_cast<int>(420 * scale * labelScale);
		const int labelHeight = static_cast<int>(labelWidth / label->GetAspectRatio());
		shader.DrawTex(label, 0, y, labelWidth, labelHeight, nullptr, &tint);
	}
	bool m_open = false;
	bool m_previousLeftDown = false;
	std::function<void(Action)> m_callback;
	std::shared_ptr<KdTexture> m_frame, m_resume, m_title, m_cursor, m_village;
	bool m_recallEnabled = true;
};
