#include "UI.h"

#include "../../main.h"

namespace
{
	const char* UIClickSoundPath = "Asset/Sounds/UI/Click.wav";

	constexpr int LevelUpCardY = 10;
	constexpr int LevelUpCardW = 300;
	constexpr int LevelUpCardH = 220;
	constexpr int LevelUpIconY = 0;
	constexpr int LevelUpIconSize = 150;
	constexpr int LevelUpCardXList[3] = { -340, 0, 340 };
	constexpr int LevelUpHoverAddW = 24;
	constexpr int LevelUpHoverAddH = 18;
	constexpr int LevelUpHoverIconAdd = 12;
	const Math::Color LevelUpHoverColor = { 1.15f, 1.15f, 1.15f, 1.0f };

	constexpr float VillageArrowEdgeX = 560.0f;
	constexpr float VillageArrowEdgeY = 300.0f;
	constexpr int VillageArrowGuideW = 96;
	constexpr int VillageArrowGuideH = 58;

	constexpr int CursorDrawW = 32;
	constexpr int CursorDrawH = 32;
}

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

void UI::PlayUIClickSound() const
{
	auto sound = KdAudioManager::Instance().Play(UIClickSoundPath);
	if (sound)
	{
		sound->SetVolume(0.5f);
	}
}

void UI::DrawLevelUpSelectUI(const LevelUpSelectUITextureSet& textures) const
{
	if (!textures.back) { return; }
	if (!textures.back->GetSRView()) { return; }

	POINT mousePos;
	GetCursorPos(&mousePos);
	ScreenToClient(Application::Instance().GetWindowHandle(), &mousePos);

	for (int i = 0; i < 3; ++i)
	{
		const int cardX = LevelUpCardXList[i];
		const bool isHover = IsMouseInSprite(mousePos, cardX, LevelUpCardY, LevelUpCardW, LevelUpCardH);
		const Math::Color* drawColor = isHover ? &LevelUpHoverColor : &kWhiteColor;
		const int cardW = LevelUpCardW + (isHover ? LevelUpHoverAddW : 0);
		const int cardH = LevelUpCardH + (isHover ? LevelUpHoverAddH : 0);
		const int iconSize = LevelUpIconSize + (isHover ? LevelUpHoverIconAdd : 0);

		KdShaderManager::Instance().m_spriteShader.DrawTex
		(
			textures.back,
			cardX,
			LevelUpCardY,
			cardW,
			cardH,
			nullptr,
			drawColor,
			{ 0.5f, 0.5f }
		);

		KdTexture* iconTex = textures.icons[i];
		if (!iconTex) { continue; }
		if (!iconTex->GetSRView()) { continue; }

		KdShaderManager::Instance().m_spriteShader.DrawTex
		(
			iconTex,
			cardX,
			LevelUpIconY,
			iconSize,
			iconSize,
			nullptr,
			drawColor,
			{ 0.5f, 0.5f }
		);
	}
}

int UI::GetClickedLevelUpSelectIndex(bool isLeftClick, bool& prevLeftClick) const
{
	if (!isLeftClick || prevLeftClick)
	{
		prevLeftClick = isLeftClick;
		return -1;
	}

	POINT mousePos;
	GetCursorPos(&mousePos);
	ScreenToClient(Application::Instance().GetWindowHandle(), &mousePos);

	for (int i = 0; i < 3; ++i)
	{
		if (IsMouseInSprite(mousePos, LevelUpCardXList[i], LevelUpCardY, LevelUpCardW, LevelUpCardH))
		{
			prevLeftClick = isLeftClick;
			return i;
		}
	}

	prevLeftClick = isLeftClick;
	return -1;
}

void UI::DrawVillageGuideUI(KdTexture* arrowTex, const Math::Vector3& playerPos, float hideRadius, KdCamera* camera) const
{
	if (!arrowTex) { return; }
	if (!arrowTex->GetSRView()) { return; }
	if (hideRadius <= 0.0f) { return; }
	if (!camera) { return; }

	Math::Vector3 toVillage = Math::Vector3::Zero - playerPos;
	toVillage.y = 0.0f;

	const float distanceSqr = toVillage.LengthSquared();
	const float hideRadiusSqr = hideRadius * hideRadius;
	if (distanceSqr <= hideRadiusSqr) { return; }
	if (distanceSqr <= 0.0001f) { return; }

	toVillage.Normalize();

	// プレイヤー位置と、村方向へ少し進めた位置を実際のカメラでスクリーン座標へ変換する。
	// 画面上でその2点を結ぶ方向を使うため、カメラ角度の符号ズレに影響されにくい。
	Math::Vector3 playerScreenPos;
	Math::Vector3 guideScreenPos;
	camera->ConvertWorldToScreenDetail(playerPos, playerScreenPos);
	camera->ConvertWorldToScreenDetail(playerPos + toVillage * 10.0f, guideScreenPos);

	Math::Vector2 guideDir =
	{
		guideScreenPos.x - playerScreenPos.x,
		guideScreenPos.y - playerScreenPos.y
	};

	if (guideDir.LengthSquared() <= 0.0001f) { return; }
	guideDir.Normalize();

	const float scaleX = VillageArrowEdgeX / std::max(fabsf(guideDir.x), 0.0001f);
	const float scaleY = VillageArrowEdgeY / std::max(fabsf(guideDir.y), 0.0001f);
	const float edgeScale = std::min(scaleX, scaleY);

	const int drawX = static_cast<int>(guideDir.x * edgeScale);
	const int drawY = static_cast<int>(guideDir.y * edgeScale);
	const float angle = atan2f(guideDir.y, guideDir.x);

	KdShaderManager::Instance().m_spriteShader.DrawTexRot
	(
		arrowTex,
		drawX,
		drawY,
		VillageArrowGuideW,
		VillageArrowGuideH,
		angle,
		nullptr,
		&kWhiteColor,
		{ 0.5f, 0.5f }
	);
}

void UI::DrawCursorUI(KdTexture* cursorTex) const
{
	if (!cursorTex) { return; }
	if (!cursorTex->GetSRView()) { return; }

	const Math::Vector2 mousePos = GetMouseSpritePos();
	KdShaderManager::Instance().m_spriteShader.DrawTex
	(
		cursorTex,
		static_cast<int>(mousePos.x),
		static_cast<int>(mousePos.y),
		CursorDrawW,
		CursorDrawH,
		nullptr,
		&kWhiteColor,
		{ 0.0f, 0.0f }
	);
}
