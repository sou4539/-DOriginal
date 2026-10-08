#include "TitleScene.h"
#include "../SceneManager.h"
#include "../../GameObject/Stage/Ground/Ground.h"
#include "../../GameObject/Stage/Village/Village.h"
#include "../../GameObject/Camera/CameraBase.h"
#include "../../GameObject/Character/Player/Player.h"
#include "../../GameObject/Character/Staff/FireStaff/FireStaff.h"
#include "../../GameObject/Character/Staff/IceStaff/IceStaff.h"
#include "../../GameObject/Character/Staff/VoltStaff/VoltStaff.h"
#include "../../main.h"

#include <filesystem>
#include <fstream>

namespace
{
	const char* ProgressSavePath = "Save/Progress.txt";
	const char* UIClickSoundPath = "Asset/Sounds/UI/Click.wav";
	const char* TitleBgmPath = "Asset/Sounds/BGM/Title/Title_BGM.wav";

	// ボタン番号の意味を明示する。重なった場合は近い中心を選ぶ。
	enum class TitleButton { None, Start, Exit };

	struct MagicUnlockSave
	{
		bool hasFire = false;
		bool hasIce = false;
		bool hasVolt = false;
	};

	class TitleCamera : public CameraBase
	{
	public:
		void Init() override
		{
			CameraBase::Init();
			UpdateCameraMatrix();
		}

		void PostUpdate() override
		{
			UpdateCameraMatrix();
		}

	private:
		void UpdateCameraMatrix()
		{
			// 以前のタイトル配置に戻し、プレイヤーの高さ2を注視する。
			Math::Vector3 playerPos = { -30.0f, 0.0f, 0.0f };
			if (auto player = m_wpTarget.lock()) { playerPos = player->GetPos(); }
			const Math::Vector3 cameraPos = { -45.0f, 8.0f, 0.0f };
			const Math::Vector3 targetPos = playerPos + Math::Vector3(0.0f, 2.0f, 0.0f);

			Math::Vector3 toTarget = targetPos - cameraPos;
			if (toTarget.LengthSquared() > 0.0001f)
			{
				toTarget.Normalize();
			}

			m_mWorld = Math::Matrix::CreateWorld(cameraPos, -toTarget, Math::Vector3::Up);
		}
	};

	// スタートボタンの表示位置。
	constexpr int StartButtonX = 400;
	constexpr int StartButtonY = -200;

	// 終了ボタンの表示位置。
	constexpr int ExitButtonX = 400;
	constexpr int ExitButtonY = -300;

	// タイトルロゴの表示位置。
	constexpr int TitleLogoX = -345;
	constexpr int TitleLogoY = 235;

	// ボタン画像の表示サイズ。
	constexpr int ButtonW = 220;
	constexpr int ButtonH = 70;
	// タイトルロゴの表示サイズ。
	constexpr int TitleLogoW = 520;
	constexpr int TitleLogoH = 170;
	// マウスカーソル画像の表示サイズ。
	constexpr int CursorDrawW = 32;
	constexpr int CursorDrawH = 32;

	const Math::Color ButtonHoverColor = { 1.15f, 1.15f, 1.15f, 1.0f };
	const Math::Color ButtonPressColor = { 0.65f, 0.65f, 0.65f, 1.0f };

	void SetCursorVisible(bool isVisible)
	{
		if (isVisible)
		{
			while (ShowCursor(TRUE) < 0) {}
		}
		else
		{
			while (ShowCursor(FALSE) >= 0) {}
		}
	}

		// 読み込めない画像はnullptrにし、描画側で安全に省略する。
	std::shared_ptr<KdTexture> LoadTitleTexture(const char* path)
	{
		auto texture = std::make_shared<KdTexture>();
		return texture->Load(path) ? texture : nullptr;
	}

	// ロゴ・ボタン・カーソルで共通の画像描画。中心座標で配置する。
	void DrawTitleTexture(const std::shared_ptr<KdTexture>& texture,
		int x, int y, int width, int height, const Math::Color* color = &kWhiteColor)
	{
		if (!texture || !texture->GetSRView()) { return; }
		KdShaderManager::Instance().m_spriteShader.DrawTex(
			texture.get(), x, y, width, height, nullptr, color, { 0.5f, 0.5f });
	}

	POINT GetClientMousePosition()
	{
		POINT position{};
		GetCursorPos(&position);
		ScreenToClient(Application::Instance().GetWindowHandle(), &position);
		return position;
	}
Math::Vector2 GetMouseSpritePos(const POINT& mousePos)
	{
		return
		{
			static_cast<float>(mousePos.x - 640),
			static_cast<float>(360 - mousePos.y)
		};
	}

	bool IsSpriteRectOverlap(float aX, float aY, int aW, int aH, int bX, int bY, int bW, int bH)
	{
		const float aHalfW = aW * 0.5f;
		const float aHalfH = aH * 0.5f;
		const float bHalfW = bW * 0.5f;
		const float bHalfH = bH * 0.5f;

		return fabsf(aX - static_cast<float>(bX)) <= aHalfW + bHalfW &&
			   fabsf(aY - static_cast<float>(bY)) <= aHalfH + bHalfH;
	}

	TitleButton GetHoveredButton(const POINT& mousePos)
	{
		const Math::Vector2 cursorPos = GetMouseSpritePos(mousePos);

		const bool isHoverStart = IsSpriteRectOverlap(cursorPos.x, cursorPos.y, CursorDrawW, CursorDrawH, StartButtonX, StartButtonY, ButtonW, ButtonH);
		const bool isHoverExit = IsSpriteRectOverlap(cursorPos.x, cursorPos.y, CursorDrawW, CursorDrawH, ExitButtonX, ExitButtonY, ButtonW, ButtonH);

		if (isHoverStart && !isHoverExit) { return TitleButton::Start; }
		if (!isHoverStart && isHoverExit) { return TitleButton::Exit; }
		if (!isHoverStart && !isHoverExit) { return TitleButton::None; }

		const float startDx = cursorPos.x - static_cast<float>(StartButtonX);
		const float startDy = cursorPos.y - static_cast<float>(StartButtonY);
		const float exitDx = cursorPos.x - static_cast<float>(ExitButtonX);
		const float exitDy = cursorPos.y - static_cast<float>(ExitButtonY);

		const float startDistSq = (startDx * startDx) + (startDy * startDy);
		const float exitDistSq = (exitDx * exitDx) + (exitDy * exitDy);

		return startDistSq <= exitDistSq ? TitleButton::Start : TitleButton::Exit;
	}

	const Math::Color* GetButtonDrawColor(bool isHover, bool leftDown)
	{
		if (!isHover) { return &kWhiteColor; }

		return leftDown ? &ButtonPressColor : &ButtonHoverColor;
	}

	std::shared_ptr<KdSoundInstance> PlayUIClickSound()
	{
		auto sound = KdAudioManager::Instance().Play(UIClickSoundPath);
		if (sound)
		{
			sound->SetVolume(0.5f);
		}

		return sound;
	}

	MagicUnlockSave LoadMagicUnlockSave()
	{
		MagicUnlockSave save;

		std::ifstream file(ProgressSavePath);
		if (!file) { return save; }

		// Statusの保存順序に合わせ、魔法の解放フラグより前の項目を読み飛ばす。
		// 保存形式を変える場合はStatus側とこの読み込みを同時に更新する。
		float dummyFloat = 0.0f;
		int dummyInt = 0;

		for (int i = 0; i < 6; ++i)
		{
			file >> dummyFloat;
		}
		file >> dummyInt;
		for (int i = 0; i < 2; ++i)
		{
			file >> dummyFloat;
		}
		file >> dummyFloat;
		for (int i = 0; i < 3; ++i)
		{
			file >> dummyInt;
		}

		if (!(file >> save.hasFire >> save.hasIce >> save.hasVolt))
		{
			return {};
		}

		return save;
	}

	void ResetProgressSave()
	{
		std::error_code error;
		std::filesystem::remove(ProgressSavePath, error);
	}
}


TitleScene::~TitleScene()
{
	if (m_titleBgm)
	{
		m_titleBgm->Stop();
		m_titleBgm = nullptr;
	}
}
void TitleScene::Event()
{
	// 終了を予約した後は、音が終わるまで新しい入力を受け付けない。
	if (m_isExitRequested)
	{
		UpdateExitRequest();
		return;
	}
	UpdateProgressReset();
	UpdateButtonInput();
}
void TitleScene::UpdateProgressReset()
{
	const bool resetDown = (GetAsyncKeyState(VK_F9) & 0x8000);
	if (resetDown && !m_prevResetDown)
	{
		ResetProgressSave();

		// セーブを初期化したので、タイトルに表示している取得済みの杖も取り除く。
		m_objList.remove_if
		(
			[](const std::shared_ptr<KdGameObject>& obj)
			{
				return std::dynamic_pointer_cast<StaffBase>(obj) != nullptr;
			}
		);
	}
	m_prevResetDown = resetDown;
}

void TitleScene::UpdateButtonInput()
{
	const POINT mousePos = GetClientMousePosition();

	const bool leftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
	const TitleButton hoverButton = GetHoveredButton(mousePos);

	if (leftDown && !m_prevLeftDown && hoverButton == TitleButton::Start)
	{
		StartGame();
	}

	if (leftDown && !m_prevLeftDown && hoverButton == TitleButton::Exit)
	{
		m_exitClickSound = PlayUIClickSound();
		m_isExitRequested = true;
	}

	m_prevLeftDown = leftDown;
}

void TitleScene::StartGame()
{
	PlayUIClickSound();
	if (m_titleBgm) { m_titleBgm->Stop(); }
	SceneManager::Instance().SetNextScene(SceneManager::SceneType::Game);
}

void TitleScene::UpdateExitRequest()
{
	if (!m_exitClickSound || m_exitClickSound->IsStopped())
	{
		PostMessage(Application::Instance().GetWindowHandle(), WM_CLOSE, 0, 0);
	}
}
void TitleScene::Init()
{
	SetCursorVisible(false);
	StartTitleBgm();
	CreateBackground();
	LoadUITextures();
}

void TitleScene::StartTitleBgm()
{
	m_titleBgm = KdAudioManager::Instance().Play(TitleBgmPath, true);
	if (m_titleBgm)
	{
		// タイトルBGMは主張しすぎないよう、通常音量の半分で流す。
		m_titleBgm->SetVolume(0.5f);
	}

}

void TitleScene::CreateBackground()
{
	std::shared_ptr<TitleCamera> camera = std::make_shared<TitleCamera>();
	camera->Init();
	m_objList.push_back(camera);

	std::shared_ptr<Ground> ground = std::make_shared<Ground>();
	m_objList.push_back(ground);

	std::shared_ptr<Village> village = std::make_shared<Village>();
	m_objList.push_back(village);

	std::shared_ptr<Player> player = std::make_shared<Player>();
	player->SetControlEnable(false);
	// タイトル用の以前のプレイヤー配置。
	player->SetPos({ -30.0f, 0.0f, 0.0f });
	player->SetAngle(DirectX::XMConvertToRadians(-90.0f));
	m_objList.push_back(player);

	// タイトルカメラもこのプレイヤーを基準に構図を決める。
	camera->SetTarget(player);
	camera->PostUpdate();
	ground->SetTarget(player);

	std::shared_ptr<FireStaff> fireStaff = std::make_shared<FireStaff>();
	std::shared_ptr<IceStaff> iceStaff = std::make_shared<IceStaff>();
	std::shared_ptr<VoltStaff> voltStaff = std::make_shared<VoltStaff>();
	const MagicUnlockSave unlocks = LoadMagicUnlockSave();

	fireStaff->SetTarget(player);
	iceStaff->SetTarget(player);
	voltStaff->SetTarget(player);

	if (unlocks.hasFire)
	{
		m_objList.push_back(fireStaff);
	}
	if (unlocks.hasIce)
	{
		m_objList.push_back(iceStaff);
	}
	if (unlocks.hasVolt)
	{
		m_objList.push_back(voltStaff);
	}

}

void TitleScene::LoadUITextures()
{
	m_startButtonTex = LoadTitleTexture("Asset/Textures/UI/Title/StartButton.png");
	m_exitButtonTex = LoadTitleTexture("Asset/Textures/UI/Title/ExitButton.png");
	m_titleLogoTex = LoadTitleTexture("Asset/Textures/UI/Title/Spell Meadow.png");
	m_cursorTex = LoadTitleTexture("Asset/Textures/UI/Cursor.png");
}

void TitleScene::DrawSprite()
{
	const POINT mousePos = GetClientMousePosition();
	const bool leftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	const TitleButton hoveredButton = GetHoveredButton(mousePos);
	const Math::Vector2 cursorPos = GetMouseSpritePos(mousePos);

	auto& shader = KdShaderManager::Instance().m_spriteShader;
	shader.Begin();
	// 描画順を固定し、カーソルをボタンより手前に表示する。
	DrawTitleTexture(m_titleLogoTex, TitleLogoX, TitleLogoY, TitleLogoW, TitleLogoH);
	DrawTitleTexture(m_startButtonTex, StartButtonX, StartButtonY, ButtonW, ButtonH,
		GetButtonDrawColor(hoveredButton == TitleButton::Start, leftDown));
	DrawTitleTexture(m_exitButtonTex, ExitButtonX, ExitButtonY, ButtonW, ButtonH,
		GetButtonDrawColor(hoveredButton == TitleButton::Exit, leftDown));
	DrawTitleTexture(m_cursorTex, static_cast<int>(cursorPos.x), static_cast<int>(cursorPos.y),
		CursorDrawW, CursorDrawH);
	shader.End();
}
