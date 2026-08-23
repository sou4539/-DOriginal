#pragma once

class UI : public KdGameObject
{
public:
	UI() {}
	~UI() override {}

protected:
	std::shared_ptr<KdTexture> LoadTexture(const char* path) const;
	Math::Vector2 GetMouseSpritePos() const;
	bool IsMouseInSprite(int centerX, int centerY, int width, int height) const;
	bool IsMouseInSprite(const POINT& mousePos, int centerX, int centerY, int width, int height) const;
};