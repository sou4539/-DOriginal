#include "main.h"

#include "Scene/SceneManager.h"

#include <filesystem>

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
namespace
{
	// exeから親フォルダーをたどり、Assetがある場所を作業ディレクトリにする。
	bool SetAssetWorkingDirectory()
	{
		// 起動方法によらずAssetとSaveの基準位置を固定する。
		wchar_t executablePath[32768] = {};
		// 日本語を含むexeの絶対パスを取得する。32768はこの配列の文字数。
		const DWORD pathLength = GetModuleFileNameW(nullptr, executablePath, 32768);
		if (pathLength > 0 && pathLength < 32768)
		{
			// まずexeのあるフォルダーから探し始める。
			auto directory = std::filesystem::path(executablePath).parent_path();
			// フォルダー操作の失敗はerrorで受け取り、例外で終了しないようにする。
			std::error_code error;
			while (!directory.empty())
			{
				if (std::filesystem::is_directory(directory / L"Asset", error))
				{
					// exeから得たUnicodeの実パスを使用する。短縮名へは変換しない。
					const bool directorySet = SetCurrentDirectoryW(directory.c_str()) != FALSE;
					const DWORD directoryError = directorySet ? ERROR_SUCCESS : GetLastError();
					const auto noisePath = directory / L"Asset/Textures/System/WhiteNoise.png";
					const auto titlePath = directory / L"Asset/Textures/UI/Title/StartButton.png";
					const bool noiseExists = GetFileAttributesW(noisePath.c_str()) != INVALID_FILE_ATTRIBUTES;
					const DWORD noiseError = noiseExists ? ERROR_SUCCESS : GetLastError();
					const bool titleExists = GetFileAttributesW(titlePath.c_str()) != INVALID_FILE_ATTRIBUTES;
					const DWORD titleError = titleExists ? ERROR_SUCCESS : GetLastError();
					if (directorySet && noiseExists && titleExists)
					{
						return true;
					}
					const std::wstring message = L"Required assets could not be found in:\n" + directory.wstring()
						+ L"\n\nDirectory error: " + std::to_wstring(directoryError)
						+ L"\nWhiteNoise error: " + std::to_wstring(noiseError)
						+ L"\nStartButton error: " + std::to_wstring(titleError)
						+ L"\n\nExecutable:\n" + executablePath;
					MessageBoxW(nullptr, message.c_str(), L"Startup asset path error", MB_OK | MB_ICONERROR);
					return false;
				}
				const auto parent = directory.parent_path();
				// ドライブの最上位では親も同じになるため、検索を終了する。
				if (parent == directory) { break; }
				directory = parent;
			}
		}
		MessageBoxW(nullptr, L"No Asset folder found beside the executable or its parent folders.",
			L"Startup asset path error", MB_OK | MB_ICONERROR);
		return false;
	}
}

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_  HINSTANCE, _In_ LPSTR , _In_ int)
{
	if (!SetAssetWorkingDirectory()) { return 1; }

	// メモリリークを知らせる
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// COM初期化
	if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
	{
		CoUninitialize();

		return 0;
	}

	// mbstowcs_s関数で日本語対応にするために呼ぶ
	setlocale(LC_ALL, "japanese");

	//===================================================================
	try
	{
		Application::Instance().Execute();
	}
	catch (const std::exception& error)
	{
		MessageBoxA(nullptr, error.what(), "Startup exception", MB_OK | MB_ICONERROR);
		CoUninitialize();
		return 1;
	}

	// COM解放
	CoUninitialize();

	return 0;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Application::KdBeginUpdate()
{
	// 入力状況の更新
	KdInputManager::Instance().Update();

	// 空間環境の更新
	KdShaderManager::Instance().WorkAmbientController().Update();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Application::KdPostUpdate()
{
	// 3DSoundListnerの行列を更新
	KdAudioManager::Instance().SetListnerMatrix(KdShaderManager::Instance().GetCameraCB().mView.Invert());
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Application::PreUpdate()
{
	SceneManager::Instance().PreUpdate();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Application::Update()
{
	SceneManager::Instance().Update();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Application::PostUpdate()
{
	SceneManager::Instance().PostUpdate();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Application::KdBeginDraw(bool usePostProcess)
{
	KdDirect3D::Instance().ClearBackBuffer();

	KdShaderManager::Instance().WorkAmbientController().Draw();

	if (!usePostProcess) return;
	KdShaderManager::Instance().m_postProcessShader.Draw();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Application::KdPostDraw()
{
	// Imguiのレンダリング
	KdDebugGUI::Instance().GuiProcess();

	// BackBuffer -> 画面表示
	KdDirect3D::Instance().WorkSwapChain()->Present(0, 0);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Application::PreDraw()
{
	SceneManager::Instance().PreDraw();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Application::Draw()
{
	SceneManager::Instance().Draw();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Application::PostDraw()
{
	// 画面のぼかしや被写界深度処理の実施
	KdShaderManager::Instance().m_postProcessShader.PostEffectProcess();

	// 現在のシーンのデバッグ描画
	SceneManager::Instance().DrawDebug();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Application::DrawSprite()
{
	SceneManager::Instance().DrawSprite();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool Application::Init(int w, int h)
{
	//===================================================================
	if (m_window.Create(w, h, "3D GameProgramming", "Window") == false) {
		MessageBoxA(nullptr, "ウィンドウ作成に失敗", "エラー", MB_OK);
		return false;
	}

	//===================================================================
	bool fullscreen = false;
//	if (MessageBoxA(m_window.GetWndHandle(), "フルスクリーンにしますか？", "確認", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES) {

	//===================================================================

	// デバイスのデバッグモードを有効にする
	bool debugDevice = false;
#ifdef _DEBUG
	debugDevice = true;
#endif

	// Direct3D初期化
	std::string errorMsg;
	if (KdDirect3D::Instance().Init(m_window.GetWndHandle(), w, h, debugDevice, errorMsg) == false) {
		MessageBoxA(m_window.GetWndHandle(), errorMsg.c_str(), "Direct3D初期化失敗", MB_OK | MB_ICONSTOP);
		return false;
	}

	// フルスクリーン設定
	if (fullscreen) {
		HRESULT hr;

		hr = KdDirect3D::Instance().SetFullscreenState(TRUE, 0);
		if (FAILED(hr))
		{
			MessageBoxA(m_window.GetWndHandle(), "フルスクリーン設定失敗", "Direct3D初期化失敗", MB_OK | MB_ICONSTOP);
			return false;
		}
	}

	//===================================================================
	KdDebugGUI::Instance().GuiInit(w, h);

	//===================================================================
	KdShaderManager::Instance().Init();

	//===================================================================
	KdAudioManager::Instance().Init();

	//===================================================================
	KdFontManager::Instance().Init(GetWindowHandle());

	//===================================================================

	return true;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Application::Execute()
{
	KdCSVData windowData("Asset/Data/WindowSettings.csv");
	const std::vector<std::string>& sizeData = windowData.GetLine(0);

	//===================================================================
	if (Application::Instance().Init(atoi(sizeData[0].c_str()), atoi(sizeData[1].c_str())) == false) {
		return;
	}

	//===================================================================

	// 時間
	m_fpsController.Init();

	// ループ
	while (1)
	{
		// 処理開始時間Get
		m_fpsController.UpdateStartTime();

		// ゲーム終了指定があるときはループ終了
		if (m_endFlag)
		{
			break;
		}

		//=========================================

		// ウィンドウのメッセージを処理する
		m_window.ProcessMessage();

		// ウィンドウが破棄されてるならループ終了
		if (m_window.IsCreated() == false)
		{
			break;
		}


		//=========================================

		KdBeginUpdate();
		{
			PreUpdate();

			Update();

			PostUpdate();
		}
		KdPostUpdate();

		//=========================================

		KdBeginDraw();
		{
			PreDraw();

			Draw();

			PostDraw();

			DrawSprite();
		}
		KdPostDraw();

		//=========================================

		m_fpsController.Update();

		std::string windowTitle = "Spell Meadow  FPS:" + std::to_string(m_fpsController.m_nowfps);
		SetWindowTextA(m_window.GetWndHandle(), windowTitle.c_str());
	}

	//===================================================================
	Release();
}

// アプリケーション終了
void Application::Release()
{
	KdInputManager::Instance().Release();

	KdShaderManager::Instance().Release();

	KdAudioManager::Instance().Release();

	KdDirect3D::Instance().Release();

	// ウィンドウ削除
	m_window.Release();
}
