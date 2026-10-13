#pragma once
#include "UI.h"

enum class InteractionAction { None, EnterCastle, ReturnToGrass, ReturnToVillage };

// ステージは操作可否を決め、このクラスは画像とレイアウトだけを担当する。
class InteractionPrompt : public UI
{
public:
	InteractionPrompt()
	{
		m_key = LoadTexture("Asset/Textures/UI/No_Push_F.png");
		m_pressedKey = LoadTexture("Asset/Textures/UI/Push_F.png");
		m_enter = LoadTexture("Asset/Textures/UI/Go_Castle.png");
		m_grass = LoadTexture("Asset/Textures/UI/Come_Back_Grass.png");
		m_village = LoadTexture("Asset/Textures/UI/Come_Back_Village.png");
	}

	void Draw(InteractionAction action) const
	{
		KdTexture* text = action == InteractionAction::EnterCastle ? m_enter.get()
			: action == InteractionAction::ReturnToGrass ? m_grass.get()
			: action == InteractionAction::ReturnToVillage ? m_village.get() : nullptr;
		KdTexture* key = (GetAsyncKeyState('F') & 0x8000) ? m_pressedKey.get() : m_key.get();
		if (!text || !key) { return; }
		constexpr int textHeight = 72;
		constexpr int keySize = 48;
		constexpr int gap = 16;
		constexpr int y = -285;
		const int textWidth = static_cast<int>(textHeight * text->GetAspectRatio());
		const int left = -(keySize + gap + textWidth) / 2;
		auto& shader = KdShaderManager::Instance().m_spriteShader;
		shader.DrawTex(key, left + keySize / 2, y, keySize, keySize);
		shader.DrawTex(text, left + keySize + gap + textWidth / 2, y, textWidth, textHeight);
	}

private:
	std::shared_ptr<KdTexture> m_key, m_pressedKey, m_enter, m_grass, m_village;
};
