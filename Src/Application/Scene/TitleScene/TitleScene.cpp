
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

namespace
{
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
			const Math::Vector3 cameraPos = { -45.0f, 8.0f, -1.0f };
			const Math::Vector3 targetPos = { -30.0f, 2.0f, -1.0f };

			Math::Vector3 toTarget = targetPos - cameraPos;
			if (toTarget.LengthSquared() > 0.0001f)
			{
				toTarget.Normalize();
			}

			m_mWorld = Math::Matrix::CreateWorld(cameraPos, -toTarget, Math::Vector3::Up);
		}
	};

	constexpr int StartButtonX = 500;
	constexpr int StartButtonY = -230;
	constexpr int ExitButtonX = 500;
	constexpr int ExitButtonY = -310;
	constexpr int ButtonW = 220;
	constexpr int ButtonH = 70;

	bool IsMouseInSprite(const POINT& mousePos, int centerX, int centerY, int width, int height)
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
}

void TitleScene::Event()
{
	POINT mousePos;
	GetCursorPos(&mousePos);
	ScreenToClient(Application::Instance().GetWindowHandle(), &mousePos);

	const bool isLeftClick = (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
	const bool isClickStartButton = IsMouseInSprite(mousePos, StartButtonX, StartButtonY, ButtonW, ButtonH);
	const bool isClickExitButton = IsMouseInSprite(mousePos, ExitButtonX, ExitButtonY, ButtonW, ButtonH);

	if (isLeftClick && !m_prevLeftClick && isClickStartButton)
	{
		SceneManager::Instance().SetNextScene
		(
			SceneManager::SceneType::Game
		);
	}

	if (isLeftClick && !m_prevLeftClick && isClickExitButton)
	{
		PostMessage(Application::Instance().GetWindowHandle(), WM_CLOSE, 0, 0);
	}

	m_prevLeftClick = isLeftClick;
}

void TitleScene::Init()
{
	std::shared_ptr<TitleCamera> camera = std::make_shared<TitleCamera>();
	camera->Init();
	m_objList.push_back(camera);

	std::shared_ptr<Ground> spGround = std::make_shared<Ground>();
	m_objList.push_back(spGround);

	std::shared_ptr<Village> spVillage = std::make_shared<Village>();
	m_objList.push_back(spVillage);

	std::shared_ptr<Player> player = std::make_shared<Player>();
	player->SetControlEnable(false);
	player->SetPos({ -30.0f, 0.0f, -1.0f });
	player->SetAngle(DirectX::XMConvertToRadians(-90.0f));
	m_objList.push_back(player);

	std::shared_ptr<FireStaff> fireStaff = std::make_shared<FireStaff>();
	std::shared_ptr<IceStaff> iceStaff = std::make_shared<IceStaff>();
	std::shared_ptr<VoltStaff> voltStaff = std::make_shared<VoltStaff>();

	fireStaff->SetTarget(player);
	iceStaff->SetTarget(player);
	voltStaff->SetTarget(player);

	fireStaff->SetAngle(0.0f);
	iceStaff->SetAngle(DirectX::XMConvertToRadians(120.0f));
	voltStaff->SetAngle(DirectX::XMConvertToRadians(240.0f));

	m_objList.push_back(fireStaff);
	m_objList.push_back(iceStaff);
	m_objList.push_back(voltStaff);

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
}

void TitleScene::DrawSprite()
{
	KdShaderManager::Instance().m_spriteShader.Begin();
	{
		if (m_startButtonTex && m_startButtonTex->GetSRView())
		{
			KdShaderManager::Instance().m_spriteShader.DrawTex
			(
				m_startButtonTex.get(),
				StartButtonX,
				StartButtonY,
				ButtonW,
				ButtonH
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
				ButtonH
			);
		}
	}
	KdShaderManager::Instance().m_spriteShader.End();
}
