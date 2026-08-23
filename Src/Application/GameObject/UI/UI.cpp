#include "UI.h"

#include "../../main.h"

std::shared_ptr<KdTexture> UI::LoadTexture(const char* path) const
{
	std::shared_ptr<KdTexture> tex = std::make_shared<KdTexture>();
	if (!tex->Load(path))
	{
		return nullptr;
	}

	return tex;
}

Math::Vector2 UI::GetMouseSpritePos() const
{
	POINT mousePos;
	GetCursorPos(&mousePos);
	ScreenToClient(Application::Instance().GetWindowHandle(), &mousePos);

	return
	{
		static_cast<float>(mousePos.x - 640),
		static_cast<float>(360 - mousePos.y)
	};
}

bool UI::IsMouseInSprite(int centerX, int centerY, int width, int height) const
{
	const Math::Vector2 mousePos = GetMouseSpritePos();

	const float halfW = width * 0.5f;
	const float halfH = height * 0.5f;

	return mousePos.x >= centerX - halfW &&
		   mousePos.x <= centerX + halfW &&
		   mousePos.y >= centerY - halfH &&
		   mousePos.y <= centerY + halfH;
}

bool UI::IsMouseInSprite(const POINT& mousePos, int centerX, int centerY, int width, int height) const
{
	const int spriteX = mousePos.x - 640;
	const int spriteY = 360 - mousePos.y;

	const int halfW = width / 2;
	const int halfH = height / 2;

	return spriteX >= centerX - halfW &&
		   spriteX <= centerX + halfW &&
		   spriteY >= centerY - halfH &&
		   spriteY <= centerY + halfH;
}