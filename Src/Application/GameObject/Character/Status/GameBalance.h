#pragma once

namespace GameBalance
{
	// 草原の敵数。SpawnAreaの各上限もGrassStageでこの数から分配する。
	inline constexpr int GrassMaxEnemies = 500;
	// プレイヤーを中心とする水平距離。安全地帯を避けて環状範囲に出現する。
	inline constexpr float SpawnMinRadius = 55.0f;
	inline constexpr float SpawnMaxRadius = 120.0f;
	// 安全地帯の外周からこの距離離れると、敵数が最大に達する。
	inline constexpr float FullSpawnDistance = 250.0f;

	// 怨念の基準能力値。現在の強化倍率をHP・接触ダメージ・経験値へ掛ける。
	inline constexpr float GrudgeInitialHp = 60.0f;
	inline constexpr float GrudgeInitialDamage = 10.0f;
	inline constexpr float GrudgeInitialExp = 40.0f;
	// 0.05は倍率の5ポイント分。割合で乗算する設定ではない。
	inline constexpr float GrudgeStrengthPerKill = 0.05f;
	// 死亡・通常出口・ポータルで退城したとき、一度だけ減算する。最低倍率は1。
	inline constexpr float GrudgeStrengthLossPerExit = 0.05f;
}
