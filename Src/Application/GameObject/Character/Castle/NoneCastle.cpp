#include "NoneCastle.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <random>

namespace
{
	// 村の中心から城の原点までの水平距離。配置距離を変える場所。
	constexpr float CastleDistance = 200.0f;
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
	m_spModel->SetModelData("Asset/Models/Objects/Character/Castle/Castle.gltf");

	// 外装モデルはまず等倍で配置。ボスマップ側の拡大率は流用しない。
	m_mWorld = Math::Matrix::CreateTranslation(LoadOrCreatePosition(villageCenter));
	// 今回は表示と配置のみ。当たり判定・入場処理は別途実装する。
}

void NoneCastle::GenerateDepthMapFromLight()
{
	if (!m_spModel) { return; }
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}
