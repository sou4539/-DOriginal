#pragma once

class UI : public KdGameObject
{
public:
	UI() {}
	~UI() override {}

protected:
	struct LevelUpSelectUITextureSet
	{
		KdTexture* back = nullptr;
		KdTexture* icons[3] = {};
	};

	std::shared_ptr<KdTexture> LoadTexture(const char* path) const;
	Math::Vector2 GetMouseSpritePos() const;
	bool IsMouseInSprite(int centerX, int centerY, int width, int height) const;
	bool IsMouseInSprite(const POINT& mousePos, int centerX, int centerY, int width, int height) const;

	void PlayUIClickSound() const;
	void DrawLevelUpSelectUI(const LevelUpSelectUITextureSet& textures) const;
	int GetClickedLevelUpSelectIndex(bool isLeftClick, bool& prevLeftClick) const;
	void DrawVillageGuideUI(KdTexture* arrowTex, const Math::Vector3& playerPos, float hideRadius, KdCamera* camera) const;
	void DrawCursorUI(KdTexture* cursorTex) const;
};
