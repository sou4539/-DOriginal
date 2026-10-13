#include "NoneCastle.h"

#include <cmath>
#include <cfloat>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <random>

namespace
{
	// 村の中心から城の原点までの水平距離。配置距離を変える場所。
	constexpr float CastleDistance = 1000.0f;
	constexpr const char* PlacementPath = "Save/CastlePlacement.txt";

	Math::Vector3 LoadOrCreatePosition(const Math::Vector3& center)
	{
		// プレイヤーの能力値とは別に保存する。シーン再入場でも再抽選しない。
		Math::Vector3 pos;
		std::ifstream input(PlacementPath);
		if (input >> pos.x >> pos.y >> pos.z)
		{
			const float dx = pos.x - center.x;
			const float dz = pos.z - center.z;
			if (std::isfinite(pos.x) && std::isfinite(pos.y) && std::isfinite(pos.z) &&
				std::abs(std::hypot(dx, dz) - CastleDistance) < 0.1f &&
				std::abs(pos.y - center.y) < 0.1f)
			{
				return pos;
			}
		}
		input.close();

		// 未保存または無効なデータなら方角だけ抽選する。距離と高さは固定。
		// 保存失敗時にも、同一起動中は同じ方角を使う。
		static const float angle = []
		{
			std::mt19937 random(std::random_device{}());
			return std::uniform_real_distribution<float>(0.0f, DirectX::XM_2PI)(random);
		}();
		pos = center + Math::Vector3(std::cos(angle) * CastleDistance, 0.0f,
			std::sin(angle) * CastleDistance);

		std::error_code error;
		std::filesystem::create_directories("Save", error);
		std::ofstream output(PlacementPath);
		output << std::setprecision(9) << pos.x << ' ' << pos.y << ' ' << pos.z << '\n';
		output.close();
		if (!output)
		{
			OutputDebugStringA("Castle placement save failed; position may change on restart.\n");
		}
		return pos;
	}
}

NoneCastle::NoneCastle(const Math::Vector3& villageCenter)
{
	m_spModel = std::make_shared<KdModelWork>();
	m_spModel->SetModelData("Asset/Models/Objects/GameObjects/Castle/NonCastle.gltf");

	// 草原の城の大きさ・向き・位置。描画と当たり判定で同じ行列を使う。
	Math::Matrix scaleMat = Math::Matrix::CreateScale(10.0f);
	Math::Matrix rotMat = Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(180.0f));
	Math::Matrix transMat = Math::Matrix::CreateTranslation(LoadOrCreatePosition(villageCenter));
	m_mWorld = scaleMat * rotMat * transMat;
	// 城モデルの形状を壁の判定として登録する。
	// TypeGroundはプレイヤーの押し戻し処理が調べる属性。
	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape("NoneCastle", m_spModel, KdCollider::TypeGround);
}

Math::Vector3 NoneCastle::GetEntrancePosition() const
{
	Math::Vector3 minimum(FLT_MAX, FLT_MAX, FLT_MAX);
	Math::Vector3 maximum(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	bool found = false;
	for (const auto& node : m_spModel->GetDataNodes())
	{
		if (!node.m_spMesh) { continue; }
		DirectX::BoundingBox bounds;
		node.m_spMesh->GetBoundingBox().Transform(bounds, node.m_worldTransform * m_mWorld);
		minimum = Math::Vector3::Min(minimum, Math::Vector3(bounds.Center) - Math::Vector3(bounds.Extents));
		maximum = Math::Vector3::Max(maximum, Math::Vector3(bounds.Center) + Math::Vector3(bounds.Extents));
		found = true;
	}
	if (!found) { return GetPos(); }
	// 実際に読み込んだモデルの前面から1離れた場所。手計算の原点補正は不要。
	return Math::Vector3((minimum.x + maximum.x) * 0.5f, GetPos().y, minimum.z - 1.0f);
}

bool NoneCastle::CanEnter(const Math::Vector3& playerPosition) const
{
	const Math::Vector3 entrance = GetEntrancePosition();
	// 入口の開口部には壁がないため、壁へのレイ命中を入場条件にしない。
	// モデル前面に沿う幅6・奥行き6・高さ3の範囲に限定する。
	return std::abs(playerPosition.x - entrance.x) <= 3.0f &&
		std::abs(playerPosition.z - entrance.z) <= 3.0f &&
		playerPosition.y >= entrance.y - 0.5f &&
		playerPosition.y <= entrance.y + 2.5f;
}

void NoneCastle::GenerateDepthMapFromLight()
{
	if (!m_spModel) { return; }
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}
