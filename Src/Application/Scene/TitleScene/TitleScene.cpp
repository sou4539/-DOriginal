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
			const Math::Vector3 cameraPos = { -45.0f, 8.0f, 0.0f };
			const Math::Vector3 targetPos = { -30.0f, 2.0f, 0.0f };

			Math::Vector3 toTarget = targetPos - cameraPos;
			if (toTarget.LengthSquared() > 0.0001f)
			{
				toTarget.Normalize();
			}

			m_mWorld = Math::Matrix::CreateWorld(cameraPos, -toTarget, Math::Vector3::Up);
		}
	};

	//スタートボタン
	constexpr int StartButtonX = 400;
	constexpr int StartButtonY = -200;

	//Exitボタン
	constexpr int ExitButtonX = 400;
	constexpr int ExitButtonY = -300;

	//タイトルロゴ
	constexpr int TitleLogoX = -345;
	constexpr int TitleLogoY = 235;

	//サイズ
	//ボタン
	constexpr int ButtonW = 220;
	constexpr int ButtonH = 70;
	//タイトル
	constexpr int TitleLogoW = 520;
	constexpr int TitleLogoH = 170;
	//カーソル
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

	int GetHoverButtonIndex(const POINT& mousePos)
	{
		const Math::Vector2 cursorPos = GetMouseSpritePos(mousePos);

		const bool isHoverStart = IsSpriteRectOverlap(cursorPos.x, cursorPos.y, CursorDrawW, CursorDrawH, StartButtonX, StartButtonY, ButtonW, ButtonH);
		const bool isHoverExit = IsSpriteRectOverlap(cursorPos.x, cursorPos.y, CursorDrawW, CursorDrawH, ExitButtonX, ExitButtonY, ButtonW, ButtonH);

		if (isHoverStart && !isHoverExit) { return 0; }
		if (!isHoverStart && isHoverExit) { return 1; }
		if (!isHoverStart && !isHoverExit) { return -1; }

		const float startDx = cursorPos.x - static_cast<float>(StartButtonX);
		const float startDy = cursorPos.y - static_cast<float>(StartButtonY);
		const float exitDx = cursorPos.x - static_cast<float>(ExitButtonX);
		const float exitDy = cursorPos.y - static_cast<float>(ExitButtonY);

		const float startDistanceSqr = (startDx * startDx) + (startDy * startDy);
		const float exitDistanceSqr = (exitDx * exitDx) + (exitDy * exitDy);

		return startDistanceSqr <= exitDistanceSqr ? 0 : 1;
	}

	const Math::Color* GetButtonDrawColor(bool isHover, bool isLeftClick)
	{
		if (!isHover) { return &kWhiteColor; }

		return isLeftClick ? &ButtonPressColor : &ButtonHoverColor;
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

void TitleScene::Event()
{
	if (m_isExitRequested)
	{
		if (!m_exitClickSound || m_exitClickSound->IsStopped())
		{
			PostMessage(Application::Instance().GetWindowHandle(), WM_CLOSE, 0, 0);
		}
		return;
	}

	const bool isDebugResetKey = (GetAsyncKeyState(VK_F9) & 0x8000);
	if (isDebugResetKey && !m_prevDebugResetKey)
	{
		ResetProgressSave();

		// 保存状態を初期化したので、タイトルに表示している取得済みの杖も消す。
		m_objList.remove_if
		(
			[](const std::shared_ptr<KdGameObject>& obj)
			{
				return std::dynamic_pointer_cast<StaffBase>(obj) != nullptr;
			}
		);
	}
	m_prevDebugResetKey = isDebugResetKey;

	POINT mousePos;
	GetCursorPos(&mousePos);
	ScreenToClient(Application::Instance().GetWindowHandle(), &mousePos);

	const bool isLeftClick = (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
	const int hoverButtonIndex = GetHoverButtonIndex(mousePos);

	if (isLeftClick && !m_prevLeftClick && hoverButtonIndex == 0)
	{
		PlayUIClickSound();

		SceneManager::Instance().SetNextScene
		(
			SceneManager::SceneType::Game
		);
	}

	if (isLeftClick && !m_prevLeftClick && hoverButtonIndex == 1)
	{
		m_exitClickSound = PlayUIClickSound();
		m_isExitRequested = true;
	}

	m_prevLeftClick = isLeftClick;
}

void TitleScene::Init()
{
	SetCursorVisible(false);

	std::shared_ptr<TitleCamera> camera = std::make_shared<TitleCamera>();
	camera->Init();
	m_objList.push_back(camera);

	std::shared_ptr<Ground> spGround = std::make_shared<Ground>();
	m_objList.push_back(spGround);

	std::shared_ptr<Village> spVillage = std::make_shared<Village>();
	m_objList.push_back(spVillage);

	std::shared_ptr<Player> player = std::make_shared<Player>();
	player->SetControlEnable(false);
	player->SetPos({ -30.0f, 0.0f, 0.0f });
	player->SetAngle(DirectX::XMConvertToRadians(-90.0f));
	m_objList.push_back(player);

	std::shared_ptr<FireStaff> fireStaff = std::make_shared<FireStaff>();
	std::shared_ptr<IceStaff> iceStaff = std::make_shared<IceStaff>();
	std::shared_ptr<VoltStaff> voltStaff = std::make_shared<VoltStaff>();
	const MagicUnlockSave magicUnlockSave = LoadMagicUnlockSave();

	fireStaff->SetTarget(player);
	iceStaff->SetTarget(player);
	voltStaff->SetTarget(player);

	if (magicUnlockSave.hasFire)
	{
		m_objList.push_back(fireStaff);
	}
	if (magicUnlockSave.hasIce)
	{
		m_objList.push_back(iceStaff);
	}
	if (magicUnlockSave.hasVolt)
	{
		m_objList.push_back(voltStaff);
	}

	m_startButtonTex = std::make_shared<KdTexture>();
	if (!m_startButtonTex->Load("Asset/Textures/UI/Title/StartButton.png"))
	{
		m_startButtonTex = nullptr;
	}

	m_exitButtonTex = std::make_shared<KdTexture>();
	if (!m_exitButtonTex->Load("Asset/Textures/UI/Title/ExitButton.png"))
	{
		m_exitButtonTex = nullptr;
	}

	m_titleLogoTex = std::make_shared<KdTexture>();
	if (!m_titleLogoTex->Load("Asset/Textures/UI/Title/Spell Meadow.png"))
	{
		m_titleLogoTex = nullptr;
	}

	m_cursorTex = std::make_shared<KdTexture>();
	if (!m_cursorTex->Load("Asset/Textures/UI/Cursor.png"))
	{
		m_cursorTex = nullptr;
	}
}

void TitleScene::DrawSprite()
{
	POINT mousePos;
	GetCursorPos(&mousePos);
	ScreenToClient(Application::Instance().GetWindowHandle(), &mousePos);

	const bool isLeftClick = (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
	const int hoverButtonIndex = GetHoverButtonIndex(mousePos);
	const bool isHoverStart = hoverButtonIndex == 0;
	const bool isHoverExit = hoverButtonIndex == 1;
	const Math::Vector2 cursorPos = GetMouseSpritePos(mousePos);

	KdShaderManager::Instance().m_spriteShader.Begin();
	{
		if (m_titleLogoTex && m_titleLogoTex->GetSRView())
		{
			KdShaderManager::Instance().m_spriteShader.DrawTex
			(
				m_titleLogoTex.get(),
				TitleLogoX,
				TitleLogoY,
				TitleLogoW,
				TitleLogoH,
				nullptr,
				&kWhiteColor,
				{ 0.5f, 0.5f }
			);
		}

		if (m_startButtonTex && m_startButtonTex->GetSRView())
		{
			KdShaderManager::Instance().m_spriteShader.DrawTex
			(
				m_startButtonTex.get(),
				StartButtonX,
				StartButtonY,
				ButtonW,
				ButtonH,
				nullptr,
				GetButtonDrawColor(isHoverStart, isLeftClick),
				{ 0.5f, 0.5f }
			);
		}

		if (m_exitButtonTex && m_exitButtonTex->GetSRView())
		{
			KdShaderManager::Instance().m_spriteShader.DrawTex
			(
				m_exitButtonTex.get(),
				ExitButtonX,
				ExitButtonY,
				ButtonW,
				ButtonH,
				nullptr,
				GetButtonDrawColor(isHoverExit, isLeftClick),
				{ 0.5f, 0.5f }
			);
		}

		if (m_cursorTex && m_cursorTex->GetSRView())
		{
			KdShaderManager::Instance().m_spriteShader.DrawTex
			(
				m_cursorTex.get(),
				static_cast<int>(cursorPos.x),
				static_cast<int>(cursorPos.y),
				CursorDrawW,
				CursorDrawH,
				nullptr,
				&kWhiteColor,
				{ 0.5f, 0.5f }
			);
		}
	}
	KdShaderManager::Instance().m_spriteShader.End();
}
