#include "CastleStage.h"

#include "../../../GameObject/Stage/Castle/Castle.h"
#include "../../../GameObject/Stage/Castle/CastleSafeArea.h"
#include "../../../GameObject/Stage/Ground/Ground.h"
#include "../../../GameObject/Camera/TPSCamera/TPSCamera.h"
#include "../../../GameObject/Character/Player/Player.h"
#include "../../../GameObject/Character/Status/Status.h"
#include "../../../GameObject/Character/Enemy/Bat/GrudgeBat.h"
#include "../../../GameObject/Character/Staff/StaffBase.h"

#include "../../SceneManager.h"

// A sealed exit near the castle spawn. Its collision and visual open together.
class CastleExit final : public KdGameObject
{
public:
	CastleExit()
	{
		m_mWorld = Math::Matrix::CreateTranslation(-30.0f, 0.0f, -65.0f);
		m_wall = std::make_shared<KdSquarePolygon>();
		m_wall->SetScale(Math::Vector2(15.0f, 15.0f));
		m_wall->Set2DObject(false);
		m_pCollider = std::make_unique<KdCollider>();
		m_pCollider->RegisterCollisionShape("ExitWall", m_wall, KdCollider::TypeGround);
		m_gate = std::make_shared<KdSquarePolygon>();
		m_gate->SetMaterial("Asset/Textures/Gate/CastleGate.png");
		m_gate->SetScale(Math::Vector2(6.0f, 8.0f));
		m_gate->Set2DObject(false);
	}

	void SetOpen(bool open)
	{
		m_open = open;
	}

	bool IsTouching(const Math::Vector3& position) const
	{
		return m_open
			&& std::abs(position.x + 30.0f) <= 3.5f
			&& std::abs(position.z + 65.0f) <= 1.0f
			&& position.y >= -0.5f && position.y <= 7.5f;
	}

	void DrawUnLit() override
	{
		auto& shader = KdShaderManager::Instance().m_StandardShader;

		// 黒い壁は常に描画する。
		shader.DrawPolygon(
			*m_wall, m_mWorld, Math::Color(0, 0, 0, 1));

		// 敵が全滅したら出口画像を重ねる。
		if (m_open)
		{
			const Math::Matrix gateWorld =
				Math::Matrix::CreateTranslation(-30.0f, 4.0f, -64.95f);

			shader.DrawPolygon(*m_gate, gateWorld);
		}
	}

	void DrawDebugGui() override
	{
		if (ImGui::Begin("Castle Exit"))
		{
			ImGui::TextUnformatted(m_open ? "Exit open: touch and press F to return to grass."
				: "Exit sealed: defeat every grudge to open.");
		}
		ImGui::End();
	}

private:
	std::shared_ptr<KdSquarePolygon> m_wall;
	std::shared_ptr<KdSquarePolygon> m_gate;
	bool m_open = false;
};

void CastleStage::Init()
{
	GameStageBase::Init();

	// 城では杖を光らせる
	for (const auto& obj : m_objList)
	{
		if (auto staff = std::dynamic_pointer_cast<StaffBase>(obj))
		{
			staff->SetLightEnable(true);
		}
	}

	auto player = m_player.lock();
	auto status = m_status.lock();
	auto camera = m_camera.lock();
	if (!player || !status || !camera) { return; }

	// 城の開始位置と復活位置。
	const Math::Vector3 spawnPos = { -30.0f, 0.0f, -50.0f };
	player->SetPos(spawnPos);
	player->SetRespawnPos(spawnPos);

	// 草原と同じGroundを使い、地面の表示と当たり判定を登録する。
	auto ground = std::make_shared<Ground>();
	ground->SetTarget(player);
	// 画像追加後、次の行を有効にして実際のファイル名を指定する。
	ground->SetGroundTextures("Asset/Models/Objects/Stage/Ground/floor.png",
		"Asset/Models/Objects/Stage/Ground/floor_NormalMap.png");
	m_objList.push_back(ground);
	player->RegistHitObject(ground);

	// 出てくるコウモリの数を取得
	const int batCount = status->GetKillBatCount();

	// コウモリの数を初期化
	status->ResetKillBatCount();

	// キャスルステージの初期化処理
	std::shared_ptr<Castle> castle = std::make_shared<Castle>();
	castle->SetTarget(player);
	m_objList.push_back(castle);

	player->RegistHitObject(castle);

	// 中央の部屋の箱形安全地帯。復活位置は変更せず、独立して編集する。
	m_objList.push_back(std::make_shared<CastleSafeArea>(player));

	// 草原で倒した分だけ怨念を出現させる。報酬や倍率はこの城の戦闘で共有。
	CreateGrudges(batCount);
	m_exit = std::make_shared<CastleExit>();
	m_exit->SetOpen(m_grudgeBattle->DefeatedCount >= m_grudgeBattle->TotalCount);
	m_objList.push_back(m_exit);
	player->RegistHitObject(m_exit);
	m_prevExitDown = (GetAsyncKeyState('F') & 0x8000) != 0;

	m_gate = std::make_shared<KdSquarePolygon>();
	m_gate->SetMaterial("Asset/Textures/Gate/CastleGate.png");
	m_gate->SetScale(Math::Vector2(6.0f, 8.0f));
	m_gate->Set2DObject(false);
}

void CastleStage::CreateGrudges(int count)
{
	auto player = m_player.lock();
	auto status = m_status.lock();
	if (!player || !status) { return; }
	m_grudgeBattle = std::make_shared<GrudgeBattleState>();
	m_grudgeBattle->TotalCount = std::max(count, 0);
	// 中央の安全な部屋を避け、周囲8か所に分散する。位置は城の配置に合わせて調整可能。
	const Math::Vector3 spawnPoints[] = {
		{-50, 2, -20}, {-30, 2, -20}, {-10, 2, -20}, {-10, 2, 0},
		{-10, 2, 20}, {-30, 2, 20}, {-50, 2, 20}, {-50, 2, 0}
	};
	for (int i = 0; i < m_grudgeBattle->TotalCount; ++i)
	{
		auto grudge = std::make_shared<GrudgeBat>(m_grudgeBattle);
		grudge->SetTarget(player);
		grudge->SetStatus(status);
		Math::Vector3 pos = spawnPoints[i % 8];
		pos.x += KdRandom::GetFloat(-3.0f, 3.0f);
		pos.z += KdRandom::GetFloat(-3.0f, 3.0f);
		grudge->SetStartPos(pos);
		m_objList.push_back(grudge);
		m_grudges.push_back(grudge);
	}
	UpdateActiveGrudges();
}

void CastleStage::UpdateActiveGrudges()
{
	// 魔法の狙い・雷の連鎖などが使う敵リストにも怨念を登録する。
	std::vector<std::weak_ptr<EnemyBase>> activeEnemies;
	for (const auto& weakGrudge : m_grudges)
	{
		if (auto grudge = weakGrudge.lock(); grudge && grudge->CanBeTargeted())
		{
			activeEnemies.push_back(grudge);
		}
	}
	SceneManager::Instance().SetActiveEnemies(activeEnemies);
}

void CastleStage::Event()
{
	GameStageBase::Event();
	const bool exitDown = (GetAsyncKeyState('F') & 0x8000) != 0;
	const bool exitPressed = exitDown && !m_prevExitDown;
	m_prevExitDown = exitDown;
	if (IsUpdatePaused()) { return; }
	if (m_exit && m_grudgeBattle)
	{
		m_exit->SetOpen(m_grudgeBattle->DefeatedCount >= m_grudgeBattle->TotalCount);
		auto player = m_player.lock();
		if (exitPressed && player && m_exit->IsTouching(player->GetPos()) && m_onExit)
		{
			m_onExit();
		}
	}
	Bat::UpdateSharedAnimation();
	UpdateActiveGrudges();
}
